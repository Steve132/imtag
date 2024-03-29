#ifndef NAIVE_SCANLINE_HPP
#define NAIVE_SCANLINE_HPP

#include "scanline_base.hpp"
#include "scanline_base.hpp"

namespace scanline_base{

// check_all verified correct result.
template<size_t W,bool mask>
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
			return r==0xFF; // r==0
		}
	}
};

/*
template<bool mask>
index_t find_next(const uint8_t* buf,index_t N){
	return find_next_nolimit<128,mask>::impl(buf,N);
}*/

}
namespace naive{
	template<bool mask>
	index_t find_next(const uint8_t* bimg,index_t N){
		for(index_t i=0;i<N;i++){
			static constexpr uint8_t TEST= mask ? 0xFF : 0x00;
			if(bimg[i]==TEST){
				return i;
			}
		}
		return N;
	}
}

// TODO: verify it checks with unoptimized method until reaching aligned ptr for bitmask comparison.
namespace naive_align64{
	template<size_t W>
	static index_t dynamic_alignof(const uint8_t* bimg){
		return static_cast<index_t>(reinterpret_cast<uintptr_t>(bimg) & (W-1));
	}
	template<bool mask>
	index_t find_next(const uint8_t* bimg,index_t Ni){
		index_t N=Ni;
		index_t offset=0;
		index_t a1=dynamic_alignof<8>(bimg);
		if(a1){
			index_t Nremain=8-a1;
			offset=naive::find_next<mask>(bimg,Nremain);
			if(offset < Nremain) {
				return offset;
			}
			N-=Nremain;
		}
		const uint64_t* bimg64=reinterpret_cast<const uint64_t*>(bimg+offset);
		index_t M=N/8;
		for(index_t i=0;i<M;i++){
			static constexpr uint64_t TEST64= (mask ? 0x0ULL : 0xFFFFFFFFFFFFFFFFULL);
			if(bimg64[i]!=TEST64){
				index_t c=offset+8*i;
				return c+naive::find_next<mask>(bimg+c,8);
			}
		}
		offset+=M*8;
		return offset+naive::find_next<mask>(bimg+offset,N-M*8);
	}
}

#endif
