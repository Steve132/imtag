#include "SegmentImageImpl.hpp"

#include<cstdint>
#include<cstdlib>
#include<limits>

template<size_t W>
struct simd_ops
{
    static bool is_aligned(const uint8_t* bimg){
        return (reinterpret_cast<uintptr_t>(bimg) & (W-1))==0;
    }

	// check for is all ones or is all zeros for size W
    template<bool mask>
    static bool is_all(const uint8_t* bimg);
    
    template<bool mask>
	static uint_fast16_t skip(
        const uint8_t* bimg,
		uint_fast16_t i,
		const uint_fast16_t C)
    {
		if(is_aligned(bimg+i)) // TODO: alignment not required?
        {
            for(;i<(C-W) && is_all<mask>(bimg+i);i+=W){}
            return i;
        }
        else if constexpr(W > 8){
			return simd_ops<W/2>::template skip<mask>(bimg,i,C);
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

// Check if all zeros or all 1s for next 32 bytes:
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
		return _mm256_testc_si256(r,o);
    }
    else
    {
        __m256i z= _mm256_setzero_si256();
		return _mm256_testc_si256(z,r);
    }
}

template<>
template<bool mask>
inline bool simd_ops<64>::is_all(const uint8_t* bimg)
{
	__m256i r1=*reinterpret_cast<const __m256i*>(bimg);
	__m256i r2=*reinterpret_cast<const __m256i*>(bimg+32);
	if constexpr(mask)
	{
		__m256i o=_mm256_set1_epi16(-1);
		__m256i r=_mm256_and_si256(r1,r2);
		return _mm256_testc_si256(r,o);
	}
	else
	{
		__m256i z= _mm256_setzero_si256();
		__m256i r=_mm256_or_si256(r1,r2);
		return _mm256_testc_si256(z,r);
	}
}
#endif

template<class impl_tag>
struct compress_scanline;

struct native_tag{};

template<>
struct compress_scanline<native_tag>{

// NOTE: This is BY FAR the performance bottleneck:
template<class MakeSeg>
static void impl(const uint8_t* bimg,const uint_fast16_t rindex,const uint_fast16_t C,MakeSeg&& msfunc){
	uint_fast16_t i=0;
	// Iterate over one row
    while(i<C){
		// Search for 1s
		uint_fast16_t beginning;
        for(;i<C;i++){
			i=simd_ops<128>::skip<false>(bimg,i,C);
            if(bimg[i] == 0xFF) {
				// Found a 1, break
				beginning=i;
                break;
            }
        }
        if(i==C) break;
		uint_fast16_t ending=C;
        for(;i<C;i++){
			i=simd_ops<128>::skip<true>(bimg,i,C);
            if(bimg[i] != 0xFF) {
				// Found a non-1, break
				ending=i;
                break;
            }
        }
		// Make the segment:
		msfunc(rindex,beginning,ending);
    }
}
};

// tags for switching optimized versions
using default_tag=native_tag;

namespace imtag{

template<class label_t>
void SegmentImageImpl<label_t>::compress_scanlines(
    const uint8_t* binary_image,
    size_t R,size_t C,
    std::vector<std::vector<seg_t>>& output_rows){
	output_rows.resize(R);

	uint_fast16_t R16 = static_cast<uint_fast16_t>(R);
	uint_fast16_t C16 = static_cast<uint_fast16_t>(C);
	#pragma omp parallel for
	for(uint_fast16_t r=0;r<R16;r++){
		// Append segments to this scanline
		label_t rlabel = 0;
		auto& rows=output_rows[r];
		rows.clear();
		compress_scanline<default_tag>::impl(binary_image+C16*r,r,C16,
			// Make segment (seg_t) function:
			[&rows,&rlabel](uint_fast16_t rind,uint_fast16_t cbegin,uint_fast16_t cend){
				rows.emplace_back(rind,cbegin,cend,rlabel++);
			}
		);
	}

	// Assign unique labels across scanlines: linearize labels now that labels assigned per row
	label_t label = 0;
	for(size_t r=0;r<R;r++){
		auto& rows=output_rows[r];
		for(auto& row : rows)
		{
			row.label = label++;
		}
	}
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}
