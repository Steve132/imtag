#ifndef NEON_SCANLINE_HPP
#define NEON_SCANLINE_HPP

#include<cstdint>
#include<cstdlib>
#include<arm_neon.h>
#include "scanline_base.hpp"

namespace neon
{

template<bool mask>
static inline bool is_all(uint8x8_t reg)
{
	if constexpr(mask)
	{
		return vminv_u8(reg)==0xFF;
	}
	else
	{
		return vmaxv_u8(reg)==0x00;
	}
}
template<bool mask>
static inline bool is_all(uint8x16_t reg)
{
	if constexpr(mask)
	{
		return vminvq_u8(reg)==0xFF;
	}
	else
	{
		return vmaxvq_u8(reg)==0x00;
	}
}
template<bool mask>
static inline bool is_all(uint8x16x2_t reg)
{
	if constexpr(mask)
	{
		uint8x16_t r=vminq_u8(reg.val[0],reg.val[1]);
		return vminvq_u8(r)==0xFF;
	}
	else
	{
		uint8x16_t r=vmaxq_u8(reg.val[0],reg.val[1]);
		return vmaxvq_u8(r)==0x00;
	}
}
template<bool mask>
static inline bool is_all(uint8x16x4_t reg)
{
	if constexpr(mask)
	{
		uint8x16_t r0=vminq_u8(reg.val[0],reg.val[1]);
		uint8x16_t r1=vminq_u8(reg.val[2],reg.val[3]);
		uint8x16_t r=vminq_u8(r0,r1);
		return vminvq_u8(r)==0xFF;
	}
	else
	{
		uint8x16_t r0=vmaxq_u8(reg.val[0],reg.val[1]);
		uint8x16_t r1=vmaxq_u8(reg.val[2],reg.val[3]);
		uint8x16_t r=vmaxq_u8(r0,r1);
		return vmaxvq_u8(r)==0x00;
	}
}

}


namespace scanline_base{

template<size_t W,bool mask>
struct check_all;

template<bool mask>
struct check_all<8, mask> {
	static bool impl(const uint8_t* a)
	{
		uint8x8_t r=vld1_u8(a);
		return neon::is_all<mask>(r);
	}
};

template<bool mask>
struct check_all<16, mask> {
	static bool impl(const uint8_t* a)
	{
		uint8x16_t r=vld1q_u8(a);
		return neon::is_all<mask>(r);
	}
};

template<bool mask>
struct check_all<32, mask> {
	static bool impl(const uint8_t* a)
	{
		uint8x16x2_t r=vld1q_u8_x2(a);
		return neon::is_all<mask>(r);
	}
};
template<bool mask>
struct check_all<64, mask> {
	static bool impl(const uint8_t* a)
	{
		uint8x16x4_t r=vld1q_u8_x4(a);
		return neon::is_all<mask>(r);
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
index_t find_next(const uint8_t* buf,index_t N){
	return find_next_nolimit<64,mask>::impl(buf,N);
}

}


#endif
