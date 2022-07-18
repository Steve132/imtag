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
template<size_t W,bool mask>
struct find_next_limit{
    static index_t impl(const uint8_t* buf){
        if(check_all<W,!mask>::impl(buf)) return W;
        index_t r=find_next_limit<W/2,mask>::impl(buf);
        if(r < W/2) return r;
        return W/2+find_next_limit<W/2,mask>::impl(buf+W/2);
    }
};


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