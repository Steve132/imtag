#ifndef OPENMP_SCANLINE_HPP
#define OPENMP_SCANLINE_HPP

#include "scanline_base.hpp"
#include <iostream>

namespace scanline_base{

// check_all verified correct result.
template<size_t W,bool mask>
struct check_all {
	static bool impl(const uint8_t* a)
	{
		if constexpr(!mask){
			uint8_t r=0x00;
			// SIMD parallelization, not multi-threaded.
			#pragma omp simd reduction(|:r)
			for (size_t i = 0 ; i < W ; i++) {
				r |= a[i];
			}
			return r==0;
		}
		else{
			uint8_t r=0xFF;
			#pragma omp simd reduction(&:r)
			for (size_t i = 0 ; i < W ; i++) {
				r &= a[i];
			}
			return r==0xFF; // r==0
		}
	}
};


template<bool mask>
index_t find_next(const uint8_t* buf,index_t N){
	return find_next_nolimit<128,mask>::impl(buf,N);
}

}

#endif
