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
			auto seg_copy=seg;
			seg_copy.label=cc_nodes[seg.label];
			components[seg_copy.label].push_back(seg_copy);
		}
	}
}

template<class label_t>
void SegmentImageImpl<label_t>::update(const uint8_t* binary_image,ConnectivitySelection cs){
    if(cs == ConnectivitySelection::VERTICAL) throw std::runtime_error("VERTICAL IS NOT IMPLEMENTED");

	compress_scanlines(binary_image,rows,columns,segments_by_row);
	size_t nsegments = 0;
	for(const auto& seg_row : segments_by_row)
		nsegments += seg_row.size();
	ds.reset(nsegments);

    switch(cs){
        case ConnectivitySelection::HORIZONTAL:
            update_compiletime_dispatch(binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>{});
            break;
        case ConnectivitySelection::CROSS:
            update_compiletime_dispatch(binary_image,cs_tag<ConnectivitySelection::CROSS>{});
            break;
        case ConnectivitySelection::VERTICAL:
            update_compiletime_dispatch(binary_image,cs_tag<ConnectivitySelection::VERTICAL>{});
            break;
        case ConnectivitySelection::EIGHT_WAY:
            update_compiletime_dispatch(binary_image,cs_tag<ConnectivitySelection::EIGHT_WAY>{});
            break;
    }
	rows_to_components();
}

template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>)
{
	// Default.  Do not union at all.
}

template<class label_t>
static inline bool overlap(const Segment<label_t>& a,const Segment<label_t>& b){
	return (a.column_start < b.column_end) && (b.column_start < a.column_end);
}
template<class label_t>
static inline bool overlap_diag(const Segment<label_t>& a,const Segment<label_t>& b) {

    return (a.column_start <= b.column_end) && (b.column_start <= a.column_end);
}


template<class label_t>
template<ConnectivitySelection cs>
void SegmentImageImpl<label_t>::update_compiletime_dispatch_connectivity(const uint8_t* binary_image)
{
	for(size_t y = 1; y < segments_by_row.size(); y++)
	{
		// look above
		auto& prev_segments = segments_by_row[y-1];
		if(!prev_segments.size())
			continue;
		auto& segments = segments_by_row[y];
		typename std::vector<Segment<label_t>>::iterator segment = segments.begin(), prev_segment = prev_segments.begin();
		while((segment != segments.end()) && (prev_segment != prev_segments.end()))
		{
			if constexpr (cs == ConnectivitySelection::EIGHT_WAY)
			{
				if(overlap_diag(*segment,*prev_segment))
					ds.unite(segment->label, prev_segment->label);
			}
			else
			{
				if(overlap(*segment,*prev_segment))
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
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::CROSS>)
{
	update_compiletime_dispatch_connectivity<ConnectivitySelection::CROSS>(binary_image);
}
template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::EIGHT_WAY>)
{
	update_compiletime_dispatch_connectivity<ConnectivitySelection::EIGHT_WAY>(binary_image);
}
template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::VERTICAL>)
{
    
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;


}
