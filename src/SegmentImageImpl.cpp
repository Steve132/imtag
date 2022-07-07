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
BoundingBox SegmentImageImpl<label_t>::bounding_box(const typename SegmentImage<label_t>::component_t& component) const
{
	BoundingBox bb;
	for(const auto& seg : component)
	{
		if(bb.top > seg.row)
			bb.top = seg.row;
		if(bb.bottom < seg.row)
			bb.bottom = seg.row;
		if(bb.left > seg.column_start)
			bb.left = seg.column_start;
		if(bb.right < seg.column_end)
			bb.right = seg.column_end;
	}
	return bb;
}

template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>)
{
	// Default.  Do not union at all.
}

template<class label_t>
template<ConnectivitySelection cs>
void SegmentImageImpl<label_t>::update_compiletime_dispatch_connectivity(const uint8_t* binary_image)
{
	for(size_t y = 1; y < segments_by_row.size(); y++)
	{
		// look above
		auto& prev_segments = segments_by_row[y-1];
		auto& segments = segments_by_row[y];
		for(auto& segment : segments)
		{
			for(auto& prev_segment : prev_segments)
			{
				if constexpr (cs == ConnectivitySelection::EIGHT_WAY)
				{
					if(segment.overlap_diag(prev_segment))
						ds.unite(segment.label, prev_segment.label);
				}
				else
				{
					if(segment.overlap(prev_segment))
						ds.unite(segment.label, prev_segment.label);
				}
			}
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
