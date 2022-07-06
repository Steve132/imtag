#include "SegmentImageImpl.hpp"

namespace imtag{

template<class label_t>
void compress_scanline(const uint8_t* bimg,size_t C,std::vector<Segment<label_t>>& out){
    using seg_t=Segment<label_t>;
}

template<class label_t>
void SegmentImageImpl<label_t>::compress_scanlines(
    const uint8_t* binary_image,
    size_t R,size_t C,
    std::vector<std::vector<seg_t>>& output_rows){
    output_rows.resize(R);
    //TODO: OpenMP?
    for(size_t r=0;r<R;r++){
        compress_scanline(binary_image+C*r,C,output_rows[r]);
    }
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}