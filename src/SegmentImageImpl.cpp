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
	for(auto& scanline : segments_by_row){
		for(auto& seg : scanline){
			// Update segment by row label to compressed label
			seg.label = cc_nodes[seg.label];
			auto seg_copy=seg;
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

template<class label_t>
void SegmentImageImpl<label_t>::remove_components(const std::vector<label_t>& labels)
{
	for(typename SegmentImage<label_t>::components_t::iterator component_it = components.begin(); component_it != components.end(); ) {
		label_t label = component_it->front().label;
		const auto& label_it = std::find(labels.begin(), labels.end(), label);
		if(label_it != labels.end())
		{
			// Erase segment in segments_by_row
			for(const auto& seg : *component_it) {
				// Erase segments in segments_by_row belonging to this component:
				auto& scanline = segments_by_row[seg.row];
				for(typename std::vector<seg_t>::iterator scanline_seg_it = scanline.begin(); scanline_seg_it != scanline.end(); ) {
					if(scanline_seg_it->label == label)
						scanline_seg_it = scanline.erase(scanline_seg_it);
					else
						scanline_seg_it++;
				}
			}
			// Erase component
			component_it = components.erase(component_it);
		}
		else
			component_it++;
	}
}


template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}
