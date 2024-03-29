#include "imtag.hpp"
#include "imtag_statistics.hpp"
#include "stb_image_library.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include "cv_comparison/cvConnectedComponentsWithStats.h"
#include <algorithm>
#include <unordered_map>
#include <thread>

template <typename FUNC>
void benchmark(FUNC f, const size_t niters = 1000)
{
	auto start = std::chrono::high_resolution_clock::now();
	for(size_t iter = 0; iter < niters; iter++)
		f();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << "Total Duration (s): " << diff.count() / niters << std::endl;
}

void compareLabelImage(const std::vector<int>& labelImage, const imtag::SegmentImage<uint16_t>& segs)
{
	// Map from imtag component labels to cv component labels:
	//std::unordered_map<uint16_t, int> imtagToCVLabel;
	std::vector<int> validLabels(segs.components().size() + 1);
	std::unordered_map<int, int> cvComponentSizes(segs.components().size() + 1);;
	validLabels.push_back(0); // Background component
	for(const auto& component : segs.components())
	{
		int cvLabel = labelImage[component[0].row * segs.width() + component[0].column_begin];
		//imtagToCVLabel[cvLabel] = component.label;
		validLabels.push_back(cvLabel);
		cvComponentSizes[cvLabel] = 0;
	}

	int missing = 0;
	for(size_t r = 0; r < segs.height(); r++)
	{
		for(size_t c = 0; c < segs.width(); c++)
		{
			// Check that all labels are corresponding to components found by imtag
			int cvLabel = labelImage[r * segs.width() + c];
			if(std::find(validLabels.begin(), validLabels.end(), cvLabel) == validLabels.end())
			{
				missing++;
			}
			// Check if imtag component is too small: count the number of pixels with label
			else
			{
				cvComponentSizes[cvLabel]++;
			}
		}
	}
	std::cout << "unmatched component pixel count: " << missing << std::endl;

	// Pixels labeled by imtag as part of component that OpenCV disagrees is part of that component
	// i.e. imtag component too big
	int disagreement = 0;
	for(const auto& component : segs.components())
	{
		int cvLabel = labelImage[component[0].row * segs.width() + component[0].column_begin];
		int componentSize = 0;
		for(const auto& seg : component)
		{
			componentSize += seg.column_end - seg.column_begin;

			// Get pixels where component is too big:
			/*
			for(int x = seg.column_begin; x < seg.column_end; x++)
			{
				if(labelImage[component[0].row * segs.width() + component[0].column_begin] != cvLabel)
					disagreement++;
			}
			*/
		}
		if(componentSize != cvComponentSizes[cvLabel])
			std::cout << "Disagreement for size of component " << component[0].label << ": " << componentSize << " vs. cv size: " << cvComponentSizes[cvLabel] << std::endl;
	}
	std::cout << "disagreeing pixels count: " << disagreement << std::endl;
}

#include <random>
static void addNoise(uint8_t* img, const size_t width, const size_t height)
{
	std::default_random_engine generator;
	std::normal_distribution<double> dist(0, .1);
	for(size_t y = 1; y < height-1; y++)
	{
		uint8_t* img_row = img + y*width;
		for(size_t x = 1; x < width-1; x++)
		{
			if(img_row[x])
				continue;
			img_row[x] = 255 * (dist(generator) > 0.35);

			// Optionally, have cross over set to noise to ensure a component
			if(img_row[x])
			{
				img_row[x-1] = 255;
				img_row[x+1] = 255;
				*(img + (y-1)*width + x) = 255;
				*(img + (y+1)*width + x) = 255;
			}
		}
	}
}

int main(int argc,char** argv)
{
	bool do_benchmark = true; //false;

	std::string fname="../../test/blobs1.png";
	if(argc > 1)
	{
		fname=argv[1];
	}
	//std::cout << "Loading: " << fname << std::endl;
	stbi::Image bwimage(fname,1);
	//addNoise(bwimage.data(), bwimage.width(), bwimage.height());
	//bwimage.write("noise.png");

	auto segs = imtag::SegmentImage<uint16_t>(bwimage.height(), bwimage.width());
	if(do_benchmark)
	{
		size_t niters = 25000;
		auto z = [&bwimage, &segs](){ segs.update(bwimage.data(), imtag::ConnectivitySelection::CROSS); };
		auto z2 = [&bwimage](){ auto segs = cvConnectedComponentsWithStats(bwimage.data(), bwimage.width(), bwimage.height(), 4); };

		// Benchmark time to allocate and close threads:
		std::array<int,6> test_out;
		auto z3 = [&test_out](){
			std::array<std::thread,6> thread_pool;
			uint_fast16_t chunk_size = 1 / thread_pool.size();

			for(uint8_t thread_ind = 0; thread_ind < thread_pool.size(); thread_ind++)
			{
				auto& thread_test_out = test_out[thread_ind];
				thread_pool[thread_ind] = std::thread(
				[&thread_test_out]()
				{
					thread_test_out = 5;
				});
			}
			for(uint8_t thread_ind = 0; thread_ind < thread_pool.size(); thread_ind++)
				thread_pool[thread_ind].join();
		};
		//std::cout << "Empty thread allocation benchmark: " << std::endl;
		//benchmark(z3, niters);
		//for(uint8_t thread_ind = 0; thread_ind < test_out.size(); thread_ind++)
		//	std::cout << test_out[thread_ind] << std::endl;
		std::cout << "Imtag benchmark: ";
		benchmark(z, niters);
		std::cout << "CV_CC benchmark: ";
		//benchmark(z2, niters);
	}
	std::vector<int> labelImage = cvConnectedComponentsWithStats(bwimage.data(), bwimage.width(), bwimage.height(), 4, false);
	compareLabelImage(labelImage, segs);

	// Debug output:
	//auto segs0 = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data());
	std::cout << "# components: " << segs.components().size() << std::endl;
	return EXIT_SUCCESS;
}
