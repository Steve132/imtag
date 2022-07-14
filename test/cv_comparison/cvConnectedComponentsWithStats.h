#ifndef CV_CONNECTEDCOMPONENTSWITHSTATS_H
#define CV_CONNECTEDCOMPONENTSWITHSTATS_H
#include <vector>
#include <cstdint>

std::vector<int> cvConnectedComponentsWithStats(const uint8_t* image, const size_t width, const size_t height, const int connectivity = 4, const bool onlyBenchmark = true);

#endif

