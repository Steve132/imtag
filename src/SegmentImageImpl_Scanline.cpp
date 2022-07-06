#include "SegmentImageImpl.hpp"

template<class impl_tag>
struct compress_scanline;

struct native_tag{};

template<>
struct compress_scanline<native_tag>{

template<class MakeSeg>
static void impl(const uint8_t* bimg,size_t rindex,size_t C,MakeSeg&& msfunc){
    size_t i=0;
    while(i<C){
        size_t b=-1;
        for(;i<C;i++){
            if(bimg[i] == 0xFF) {
                b=i;
                break;
            }
        }
        if(i==C) break;
        size_t e=C;
        for(;i<C;i++){
            if(bimg[i] != 0xFF) {
                e=i;
                break;
            }
        }
        msfunc(rindex,b,e);
    }
}

};

using default_tag=native_tag;

namespace imtag{

template<class label_t>
void SegmentImageImpl<label_t>::compress_scanlines(
    const uint8_t* binary_image,
    size_t R,size_t C,
    std::vector<std::vector<seg_t>>& output_rows){
    output_rows.resize(R);
    //TODO: OpenMP?
    for(size_t r=0;r<R;r++){
        auto& rows=output_rows[r];
        compress_scanline<default_tag>::impl(binary_image+C*r,r,C,
            [&rows](size_t rind,size_t cbegin,size_t cend){
                rows.emplace_back(rind,cbegin,cend,0); //TODO: label
            }
        );
    }
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}