#ifndef SCANLINES_INDEX_HPP
#define SCANLINES_INDEX_HPP

#if defined(__ARM_NEON)
#include "neon_scanline.hpp"
#elif defined(__AVX2__)
#include "avx2_scanline.hpp"
#elif defined(_OPENMP)
#include "omp_scanline.hpp"
#else
#include "naive_scanline.hpp"
#endif

#endif
