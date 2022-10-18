#include <iostream>
#include<cstdint>

bool reduce(const uint8_t* a)
{
    static const unsigned N=16;
    #pragma omp simd reduction(|:r)
    for (size_t i = 0 ; i < W ; i++) {
        r |= a[i];
    }
    return r==0;
}
