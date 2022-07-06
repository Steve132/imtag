#include "SegmentImageImpl.hpp"
#include <stdexcept>

namespace imtag{



template<class label_t>
void SegmentImageImpl<label_t>::rows_to_components(){
    cc_nodes.resize(ds.size());
    size_t M=ds.compressed_freeze(cc_nodes.data());
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
}

template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>)
{
    
}
template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::CROSS>)
{
    
}
template<class label_t>
void SegmentImageImpl<label_t>::update_compiletime_dispatch
(const uint8_t* binary_image,cs_tag<ConnectivitySelection::EIGHT_WAY>)
{
    
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