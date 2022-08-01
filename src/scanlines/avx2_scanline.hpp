#ifndef AVX2_SCANLINE_HPP
#define AVX2_SCANLINE_HPP

#include<cstdint>
#include<cstdlib>
#include "scanline_base.hpp"
#include "sse4_scanline.hpp"
#include<immintrin.h>

namespace avx2
{
// avx2: _mm256_and_si256, _mm256_or_si256 is_all(__m256i), _mm256_extracti128_si256 find_next(__m128i)
// avx: _mm256_testc_si256, _mm256_set1_epi16, _mm256_setzero_si256 is_all(__m256i), _mm256_lddqu_si256 check_all<32+>

template<bool mask>
static inline bool is_all(const __m256i r)
{
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

template<bool mask>
static inline bool is_all(const __m256i r1, const __m256i r2)
{
	if constexpr(mask)
	{
		__m256i r=_mm256_and_si256(r1,r2);
		return is_all<mask>(r);
	}
	else
	{
		__m256i r=_mm256_or_si256(r1,r2);
		return is_all<mask>(r);
	}
}
template<bool mask>
static inline bool is_all(const __m256i r0, const __m256i r1, const __m256i r2, const __m256i r3){
	if constexpr(mask)
	{
		__m256i t0=_mm256_and_si256(r0,r1);
		__m256i t1=_mm256_and_si256(r2,r3);
		__m256i t=_mm256_and_si256(t0,t1);
		return is_all<mask>(t);
	}
	else
	{
		__m256i t0=_mm256_or_si256(r0,r1);
		__m256i t1=_mm256_or_si256(r2,r3);
		__m256i t=_mm256_or_si256(t0,t1);
		return is_all<mask>(t);
	}
}

template<bool mask>
static inline index_t find_next(const __m256i r){
	if(is_all<!mask>(r)) return 32;
	index_t i=find_next<mask>(_mm256_extracti128_si256(r,0));
	if(i < 16) return i;
	return 16+find_next<mask>(_mm256_extracti128_si256(r,1));
};

template<bool mask>
static inline index_t find_next(const __m256i r0,const __m256i r1){
	if(is_all<!mask>(r0,r1)) return 32;
	index_t i=find_next<mask>(r0);
	if(i < 32) return i;
	return 32+find_next<mask>(r1);
};

template<bool mask>
static inline index_t find_next(const __m256i r0,const __m256i r1,const __m256i r2,const __m256i r3){
	if(is_all<!mask>(r0,r1,r2,r3)) return 128; //TODO reuse intermediate results for and tests
	index_t i=find_next<mask>(r0,r1);
	if(i < 64) return i;
	return 64+find_next<mask>(r2,r3);
};

// Experiments suggest prefetch makes no difference for performance.
static inline void prefetch(const uint8_t* b)
{
	//#define PREFETCH
	#ifdef PREFETCH
	_mm_prefetch(b, _MM_HINT_T0);
	#endif
}

}

namespace scanline_base{

template<bool mask>
struct check_all<32, mask> {
	static bool impl(const uint8_t* a)
	{
		avx2::prefetch(a);
		__m256i r=_mm256_lddqu_si256((const __m256i*)a);
		return avx2::is_all<mask>(r);
	}
};

template<bool mask>
struct check_all<64, mask> {
	static bool impl(const uint8_t* a)
	{
		avx2::prefetch(a);
		__m256i r0=_mm256_lddqu_si256((const __m256i*)a);
		__m256i r1=_mm256_lddqu_si256((const __m256i*)(a+32));
		return avx2::is_all<mask>(r0,r1);
	}
};

template<bool mask>
struct check_all<128, mask> {
	static bool impl(const uint8_t* a)
	{
		avx2::prefetch(a);
		__m256i r0=_mm256_lddqu_si256((const __m256i*)a);
		__m256i r1=_mm256_lddqu_si256((const __m256i*)(a+32));
		__m256i r2=_mm256_lddqu_si256((const __m256i*)(a+64));
		__m256i r3=_mm256_lddqu_si256((const __m256i*)(a+96));
		return avx2::is_all<mask>(r0,r1,r2,r3);
	}
};

#ifdef DEBUG_AVX_FIND_NEXT
template<bool mask>
struct find_next_limit<32,mask>{
	static index_t impl(const uint8_t* buf){
		__m256i r=_mm256_lddqu_si256((const __m256i*)buf);
		return avx2::find_next<mask>(r);
	}
};

template<bool mask>
struct find_next_limit<64,mask>{
	static index_t impl(const uint8_t* buf){
		__m256i r0=_mm256_lddqu_si256((const __m256i*)buf);
		__m256i r1=_mm256_lddqu_si256((const __m256i*)(buf+32));
		return avx2::find_next<mask>(r0,r1);
	}
};

template<bool mask>
struct find_next_limit<128,mask>{
	static index_t impl(const uint8_t* buf){
		__m256i r0=_mm256_lddqu_si256((const __m256i*)buf);
		__m256i r1=_mm256_lddqu_si256((const __m256i*)(buf+32));
		__m256i r2=_mm256_lddqu_si256((const __m256i*)(buf+64));
		__m256i r3=_mm256_lddqu_si256((const __m256i*)(buf+96));
		return avx2::find_next<mask>(r0,r1,r2,r3);
	}
};
#endif

}

#endif
