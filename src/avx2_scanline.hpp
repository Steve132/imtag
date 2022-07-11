#include<cstdint>
#include<cstdlib>
#include<immintrin.h>

using index_t=uint_fast16_t;

template<size_t W>
static bool is_aligned(const uint8_t* bimg){
    return (reinterpret_cast<uintptr_t>(bimg) & (W-1))==0;
}
template<bool mask>
static inline bool is_all(uint64_t r);


template<bool mask>
static inline bool is_all(uint64_t r)
{
    if constexpr(mask){
        return r==0xFFFFFFFFFFFFFFFFULL;
    }
    else{
        return r==0;
    }
}

template<size_t W,bool mask>
struct find_next_limit;

template<size_t W,bool mask>
struct find_next_nolimit{
    static index_t impl(const uint8_t* buf,index_t N){
        index_t i=0;
        for(;(i+W)<N;){
            index_t r=find_next_limit<W,mask>::impl(buf+i);
            i+=r;
            if(r < W) return i;
        }
        return find_next_nolimit<W/2,mask>::impl(buf+i,N-i);
    }
};
template<size_t W,bool mask>
struct find_next_limit{
    static index_t impl(const uint8_t* buf){
        index_t r=find_next_limit<W/2,mask>::impl(buf);
        if(r < W) return r;
        return W+find_next_limit<W/2,mask>::impl(buf+W);
    }
};

template<bool mask>
struct find_next_nolimit<8,mask>{
    static index_t impl(const uint8_t* buf,index_t N){
        index_t i;
        for(i=0;i<N;i++){
            if constexpr(mask){
                if(buf[i]){
                    return i;
                }
            }
            else{
                if(!buf[i]){
                    return i;
                }
            }
        }
        return i;
    }
};

template<bool mask>
struct find_next_limit<8,mask>{
    static index_t impl(const uint8_t* buf){
        index_t i;
        for(i=0;i<8;i++){
            if constexpr(mask){
                if(buf[i]){
                    return i;
                }
            }
            else{
                if(!buf[i]){
                    return i;
                }
            }
        }
        return 8;
    }
};

template<bool mask>
static inline index_t find_next(uint64_t r){
    if(is_all<!mask>(r)) return 8;
    for(index_t i=0;i<8;i++){
        static constexpr uint64_t TEST=mask ? 0x00 : 0xFF;
        if(((r >> (8*i)) & 0xFF)==TEST){
            return i;
        }
    }
    return 8;
}

// Check if all zeros or all 1s for next 32 bytes:
#include<immintrin.h>

template<bool mask>
static inline bool is_all(__m128i r);

template<bool mask>
static inline bool is_all(__m256i r);

template<bool mask>
static inline bool is_all(__m256i r0,__m256i r1);

template<bool mask>
static inline bool is_all(__m256i r0,__m256i r1,__m256i r2,__m256i r3);

template<bool mask>
static inline bool is_all(__m128i r)
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
static inline bool is_all(__m256i r)
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
static inline bool is_all(__m256i r1,__m256i r2)
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
static inline bool is_all(__m256i r0,__m256i r1,__m256i r2,__m256i r3){
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
static inline index_t find_next(__m128i r){
    if(is_all<!mask>(r)) return 16;
    index_t i=find_next<mask>(_mm_extract_epi64(r,0));
    if(i < 8) return i;
    return 8+find_next<mask>(_mm_extract_epi64(r,1));
}

template<bool mask>
static inline index_t find_next(__m256i r){
    if(is_all<!mask>(r)) return 32;
    index_t i=find_next<mask>(_mm256_extracti128_si256(r,0));
    if(i < 16) return i;
    return 16+find_next<mask>(_mm256_extracti128_si256(r,1));
};

template<bool mask>
static inline index_t find_next(__m256i r0,__m256i r1){
    if(is_all<!mask>(r0,r1)) return 32;
    index_t i=find_next<mask>(r0);
    if(i < 32) return i;
    return 32+find_next<mask>(r1);
};
template<bool mask>
static inline index_t find_next(__m256i r0,__m256i r1,__m256i r2,__m256i r3){
    if(is_all<!mask>(r0,r1,r2,r3)) return 128; //TODO reuse intermediate results for and tests
    index_t i=find_next<mask>(r0,r1);
    if(i < 64) return i;
    return 64+find_next<mask>(r2,r3);
};

template<bool mask>
struct find_next_limit<16,mask>{
    static index_t impl(const uint8_t* buf){
        __m128i r=_mm_lddqu_si128((const __m128i*)buf);
        return find_next<mask>(r);
    }
};


template<bool mask>
struct find_next_limit<32,mask>{
    static index_t impl(const uint8_t* buf){
        __m256i r=_mm256_lddqu_si256((const __m256i*)buf);
        return find_next<mask>(r);
    }
};

template<bool mask>
struct find_next_limit<64,mask>{
    static index_t impl(const uint8_t* buf){
        __m256i r0=_mm256_lddqu_si256((const __m256i*)buf);
        __m256i r1=_mm256_lddqu_si256((const __m256i*)(buf+32));
        return find_next<mask>(r0,r1);
    }
};
template<bool mask>
struct find_next_limit<128,mask>{
    static index_t impl(const uint8_t* buf){
        __m256i r0=_mm256_lddqu_si256((const __m256i*)buf);
        __m256i r1=_mm256_lddqu_si256((const __m256i*)(buf+32));
        __m256i r2=_mm256_lddqu_si256((const __m256i*)(buf+64));
        __m256i r3=_mm256_lddqu_si256((const __m256i*)(buf+96));
        
        return find_next<mask>(r0,r1);
    }
};

template<bool mask>
index_t find_next(const uint8_t* buf,index_t N){
    return find_next_nolimit<32,mask>::impl(buf,N);
}
