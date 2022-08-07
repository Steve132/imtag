#ifndef SSE4_SCANLINE_HPP
#define SSE4_SCANLINE_HPP

#include<cstdint>
#include<cstdlib>
#include "scanline_base.hpp"
#include<immintrin.h>

namespace sse4
{
// sse4.1: _mm_testc_si128 is_all(__m128i), _mm_extract_epi64 find_next(__m128i)
// sse3: _mm_lddqu_si128 check_all<16>
// sse2: _mm_setzero_si128, _mm_set1_epi16 is_all(__m128i)

template<bool mask>
static inline bool is_all(const __m128i r)
{
	if constexpr(mask)
	{
		__m128i o=_mm_set1_epi16(-1);
		return _mm_testc_si128(r,o);
	}
	else
	{
		__m128i z= _mm_setzero_si128();
		return _mm_testc_si128(z,r);
	}
}

template<bool mask>
static inline bool is_all(const uint64_t r)
{
	if constexpr(mask){
		return r==0xFFFFFFFFFFFFFFFFULL;
	}
	else{
		return r==0;
	}
}

// mask = 1 Search for 1s i.e. beginning of segment
// mask = 0 Search for 0s i.e. end of segment
// little-endian: search from the right to the left for byte offset
template<bool mask>
static inline index_t find_next(uint64_t r){
	if(is_all<!mask>(r)) return 8;
	if constexpr(!mask)
	{
		r = ~r;
	}
	return (__builtin_ctzll(r) / 8); //TODO: visual studio version // c++20 std::countr_zero
}

template<bool mask>
static inline index_t find_next(const __m128i r){
	if(is_all<!mask>(r)) return 16;
	index_t i=find_next<mask>(_mm_extract_epi64(r,0));
	if(i < 8) return i;
	return 8+find_next<mask>(_mm_extract_epi64(r,1));
}

}

namespace scanline_base{

template<bool mask>
struct check_all<16, mask> {
	static bool impl(const uint8_t* a)
	{
		__m128i r=_mm_lddqu_si128((const __m128i*)a);
		return sse4::is_all<mask>(r);
	}
};

template<size_t W, bool mask>
struct check_all {
	static bool impl(const uint8_t* a)
	{
		if constexpr(!mask){
			uint8_t r=0x00;
			for (size_t i = 0 ; i < W ; i++) {
				r |= a[i];
			}
			return r==0;
		}
		else{
			uint8_t r=0xFF;
			for (size_t i = 0 ; i < W ; i++) {
				r &= a[i];
			}
			return r==0xFF;
		}
	}
};

template<bool mask>
struct find_next_limit<16,mask>{
	static index_t impl(const uint8_t* buf){
		__m128i r=_mm_lddqu_si128((const __m128i*)buf);
		return sse4::find_next<mask>(r);
	}
};

#ifndef __AVX2__

#endif

}

#endif
