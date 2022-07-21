#ifndef SVE_SCANLINE_HPP
#define SVE_SCANLINE_HPP

// Select ARMv64 gcc12 or Clang and flags: -Ofast -std=c++17 -march=armv8.2-a+sve

#include<cstdint>
#include<cstdlib>
#include "scanline_base.hpp"

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>

namespace sve
{

// Check if entire block is 1s (true if all ones)
static inline bool check_block_all_ones(const svuint8_t& vals, const svbool_t select)
{
	// look for 0 with min of vector:
	uint8_t min = svminv_u8(select, vals);
	return min == 0xFF;
}

static inline index_t naive_find_next_zero(const uint8_t* buf, const index_t N)
{
	for(index_t i = 0; i < N; i++)
	{
		if(buf[i] != 0xFF)
			return i;
	}
	return N;
}

// Check if entire block is 1s
index_t find_next_zero(const uint8_t* buf, const size_t N)
{
	uint64_t i = 0;
	uint64_t W = svcntw(); // dynamic lane width! i.e. W
	svbool_t pred;
	svuint8_t vals, svres;
	// loop condition: i < N (sets bit values = 1 if < N)
	pred = svwhilelt_b8( i, (uint64_t)N);
	// while(pred is all 1s):
	while(svptest_first(svptrue_b8(), pred)) {
		// load buf[i]
		vals = svld1_u8(pred, &buf[i]);

		if(!check_block_all_ones(vals, pred))
		{
			return naive_find_next_zero(buf + i, N - i);
		}
		i += W;
		// loop condition
		pred = svwhilelt_b8( i, (uint64_t)N);
	}
	return N;
}

}

#endif

#endif
