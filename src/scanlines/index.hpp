#ifndef SCANLINES_INDEX_HPP
#define SCANLINES_INDEX_HPP

#if defined(__ARM_NEON) && defined(__aarch64__)
#include "neon_scanline.hpp"
#elif defined(__AVX2__)
#include "avx2_scanline.hpp"
#elif defined(__SSE3__) && defined( __SSE4_1__ )
#include "sse4_scanline.hpp"
#elif defined(_OPENMP)
#include "omp_scanline.hpp"
#else
#include "naive_scanline.hpp"
#endif

#endif
