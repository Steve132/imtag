#include "SegmentImageImpl.hpp"
#include <stdexcept>
#include <unordered_map> // only for row_to_components temp method workaround until compressed_freeze works inside disjoint_set

namespace imtag{

template<class label_t>
void SegmentImageImpl<label_t>::rows_to_components(){
	cc_nodes.resize(ds.size());
	size_t M = ds.compressed_freeze(cc_nodes.data());
	for(auto& component : components)
		component.clear();
	components.resize(M);
	for(const auto& scanline : segments_by_row){
		for(const auto& seg : scanline){
			// Update segment label to compressed label
			auto seg_copy=seg;
			seg_copy.label=cc_nodes[seg.label];
			components[seg_copy.label].push_back(seg_copy);
		}
	}
}

template<class label_t>
void SegmentImageImpl<label_t>::update(const uint8_t* binary_image,ConnectivitySelection cs){
   	compress_scanlines(binary_image,rows,columns,segments_by_row);
	
	update_connectivity(cs);
}

template<class label_t>
void SegmentImageImpl<label_t>::update_connectivity(ConnectivitySelection cs){
	switch(cs){
        case ConnectivitySelection::HORIZONTAL:
            update_connectivity<ConnectivitySelection::HORIZONTAL>();
            break;
        case ConnectivitySelection::CROSS:
            update_connectivity<ConnectivitySelection::CROSS>();
            break;
       // case ConnectivitySelection::VERTICAL:
        //    update_compiletime_dispatch(cs_tag<ConnectivitySelection::VERTICAL>{});
        //    break;
        case ConnectivitySelection::EIGHT_WAY:
            update_connectivity<ConnectivitySelection::EIGHT_WAY>();
            break;
    };

	rows_to_components();
}

struct overlap_result_t{
	bool asLbe;
	bool bsLae;
};

template<ConnectivitySelection cs,class label_t>
static inline overlap_result_t overlap2(const Segment<label_t>& a,const Segment<label_t>& b){
	if constexpr (cs == ConnectivitySelection::EIGHT_WAY){
		return overlap_result_t{(a.column_begin <= b.column_end),(b.column_begin <= a.column_end)};
	}
	else{
		return overlap_result_t{(a.column_begin < b.column_end),(b.column_begin < a.column_end)};
	}
}

template<ConnectivitySelection cs,class label_t>
static inline bool overlap(const Segment<label_t>& a,const Segment<label_t>& b){
	if constexpr (cs == ConnectivitySelection::EIGHT_WAY){
		return (a.column_begin <= b.column_end) && (b.column_begin <= a.column_end);
	}
	else{
		return (a.column_begin < b.column_end) && (b.column_begin < a.column_end);
	}
}

template<class label_t>
template<ConnectivitySelection cs>
void SegmentImageImpl<label_t>::update_connectivity()
{
	if constexpr(cs==ConnectivitySelection::HORIZONTAL){
		return;
	}
	static constexpr size_t max_nsegments = std::numeric_limits<label_t>::max();

	size_t nsegments = 0;
	for(const auto& seg_row : segments_by_row)
		nsegments += seg_row.size(); //Todo should this techinically actually be the max label because of potentially nonlinear labeling?
	if(nsegments > max_nsegments)
		throw std::runtime_error("Too many segments found for label type.  Increase size of label type for SegmentImage.");
	ds.reset(nsegments);
	if(nsegments == 0)
		return;

	for(size_t y = 1; y < segments_by_row.size(); y++)
	{
		// look above
		const auto& prev_segments = segments_by_row[y-1];
		if(!prev_segments.size())
			continue;
		const auto& segments = segments_by_row[y];
		typename std::vector<Segment<label_t>>::const_iterator segment = segments.begin(), prev_segment = prev_segments.begin();
		while((segment != segments.end()) && (prev_segment != prev_segments.end()))
		{
			if(overlap<cs>(*segment,*prev_segment)){
				ds.unite(segment->label, prev_segment->label);
			}

			if(prev_segment->column_end < segment->column_end)
				prev_segment++;
			else
				segment++;
		}
	}
}


template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}
