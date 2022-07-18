#ifndef SCANLINE_BASE_HPP
#define SCANLINE_BASE_HPP

#include<cstdint>
#include<cstdlib>

using index_t=uint_fast16_t;

namespace scanline_base{

template<size_t W,bool mask>
struct check_all;

template<size_t W,bool mask>
struct find_next_limit;

// TODO: Alternative: Prefix sum ones test reduction?

// Split up window for search into cache-line sizes to optimize search
// No run-time recursion: just compile-time recursion on fixed width window sizes.
template<size_t W,bool mask>
struct find_next_nolimit{
    static index_t impl(const uint8_t* buf,index_t N){
        index_t M=N/W;
        for(index_t i=0;i<M;i++){
            index_t r=find_next_limit<W,mask>::impl(buf+i*M);
            if(r < W) return i*M+r;
        }
        return M*W+find_next_nolimit<W/2,mask>::impl(buf+M*W,N-W*M);
    }
};

// check_all is a dumb check for ALL of the same value: doesn't give index (for fast optimization)
// Say window size W=128.  Say looking for next 0 in a window of 1s (mask=false).
// Use check_all to see if they're ALL true.  If they are, return W.
// If check_all finds a 0 in the the next window, find where it is with a BINARY SEARCH.
template<size_t W,bool mask>
struct find_next_limit{
    static index_t impl(const uint8_t* buf){
        if(check_all<W,!mask>::impl(buf)) return W;
        index_t r=find_next_limit<W/2,mask>::impl(buf);
        if(r < W/2) return r;
        return W/2+find_next_limit<W/2,mask>::impl(buf+W/2);
    }
};

// Base case for binary search for find_next_limit with |W| = 0
template<bool mask>
struct find_next_limit<0,mask>{
    static index_t impl(const uint8_t* buf){
        return 0;
    }
};

template<bool mask>
struct find_next_nolimit<0,mask>{
    static index_t impl(const uint8_t* buf,size_t N){
        return 0;
    }
};

}

#endif
