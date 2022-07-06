#include "SegmentImageImpl.hpp"

#include<cstdint>
#include<cstdlib>

template<size_t W>
struct simd_ops
{
    static bool is_aligned(const uint8_t* bimg){
        return (reinterpret_cast<uintptr_t>(bimg) & (W-1))==0;
    }
    template<bool mask>
    static bool is_all(const uint8_t* bimg);
    
    template<bool mask>
    static size_t skip_align(
        const uint8_t* bimg,
        size_t i,
        const size_t C)
    {
        if(is_aligned(bimg+i))
        {
            for(;i<(C-W) && is_all<mask>(bimg+i);i+=W){}
            return i;
        }
        else if constexpr(W > 8){
            return simd_ops<W/2>::template skip_align<mask>(bimg,i,C);
        }
        else{
            return i;
        }
    }
};

template<>
template<bool mask>
inline bool simd_ops<8>::is_all(const uint8_t* bimg){
    uint64_t r=*reinterpret_cast<const uint64_t*>(bimg);
    if constexpr(mask){
        return r==0xFFFFFFFFFFFFFFFFULL;
    }
    else{
        return r==0;
    }
}
template<size_t W>
template<bool mask>
inline bool simd_ops<W>::is_all(const uint8_t* bimg){
    return 
    simd_ops<W/2>::template is_all<mask>(bimg) && 
    simd_ops<W/2>::template is_all<mask>(bimg+W/2);
}

#ifdef __AVX2__
#include<immintrin.h>

template<>
template<bool mask>
inline bool simd_ops<32>::is_all(const uint8_t* bimg)
{
    __m256i r=*reinterpret_cast<const __m256i*>(bimg);
    if constexpr(mask)
    {
        __m256i o=_mm256_set1_epi16(-1);
        return _mm256_testc_si256 (r,o);
    }
    else
    {
        __m256i z= _mm256_setzero_si256();
        return _mm256_testc_si256 (z,r);
    }
}

#endif


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
            i=simd_ops<128>::skip_align<false>(bimg,i,C);
            if(bimg[i] == 0xFF) {
                b=i;
                break;
            }
        }
        if(i==C) break;
        size_t e=C;
        for(;i<C;i++){
            i=simd_ops<128>::skip_align<true>(bimg,i,C);
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