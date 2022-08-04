#include "imtag.hpp"
#include "imtag_statistics.hpp"
#include "stb_image_library.h"
#include <chrono>
#include <iostream>
#include <cstring>
#include "cv_comparison/cvConnectedComponentsWithStats.h"

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

#include <random>
static void addNoise(uint8_t* img, const size_t width, const size_t height)
{
	std::default_random_engine generator(100);
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
	/*
	uint64_t r = 0x00FFFFFFFFFFFFFFULL;//0xFFFFFFFFFFFFFFFFULL;
	std::cout << "leading zeros: " << __builtin_clzll(r) << std::endl;
	// find_next<1> is AFTER the leading zeros
	std::cout << "find_next<1>: " << (__builtin_clzll(r) >> 3) << std::endl;

	r = 0x0000FFFFFFFFFFFFULL;
	std::cout << "leading zeros: " << __builtin_clzll(r) << std::endl;
	// find_next<1> is AFTER the leading zeros
	std::cout << "find_next<1>: " << (__builtin_clzll(r) >> 3) << std::endl;
	//  __builtin_clzll: Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined.
	//  __builtin_ctzll: Returns the number of trailing 0-bits in x, starting at the least significant bit position. If x is 0, the result is undefined.

	r = 0x000000FFFFFFFFFFULL;
	std::cout << "leading zeros: " << __builtin_clzll(r) << std::endl;
	// find_next<1> is AFTER the leading zeros
	std::cout << "find_next<1>: " << (__builtin_clzll(r) >> 3) << std::endl;
	std::cout << "find_next<0>: " << __builtin_ctzll(r) << std::endl;

	r = 0xFFFFFFFFFFFFFFFFULL;
	bool mask = 1;
	std::cout << "mask: " << mask << std::endl;
	return 0;
	*/
	bool do_benchmark = true; //false;

	std::string fname="../../test/blobs1.png";
	if(argc > 1)
	{
		fname=argv[1];
	}
	std::cout << "Loading: " << fname << std::endl;
	stbi::Image bwimage(fname,1);
	//std::cout << "With Noise: " << std::endl;
	//addNoise(bwimage.data(), bwimage.width(), bwimage.height());
	//bwimage.write("test.png");

	using label_t = uint32_t;
	auto segs = imtag::SegmentImage<label_t>(bwimage.height(), bwimage.width());
	if(do_benchmark)
	{
		size_t niters = 1;
		auto z = [&bwimage, &segs](){ segs.update(bwimage.data(), imtag::ConnectivitySelection::CROSS); };
		//auto z = [&bwimage](){ auto segs = imtag::bwlabel<label_t>(bwimage.height(), bwimage.width(), bwimage.data()); };
		benchmark(z, niters);
	}

	// Debug output:
	//auto segs0 = imtag::bwlabel<label_t>(bwimage.height(), bwimage.width(), bwimage.data());
	std::cout << "# components: " << segs.components().size() << std::endl;

	stbi::Image maskImage(bwimage.width(), bwimage.height(), 1);
	segs.to_mask_image(maskImage.data());
	maskImage.write("mask.png");

	std::vector<label_t> labelImage0(bwimage.width() * bwimage.height());
	segs.to_label_image(labelImage0.data());
	if(segs.components().size() > 1)
	{
		const auto& first_component_segment = segs.components()[1][0];
		const label_t label_image_first_component = (labelImage0.data() + first_component_segment.row * bwimage.width() + first_component_segment.column_begin)[0];
		if(first_component_segment.label != label_image_first_component)
			std::cout << "error with to_label_image." << std::endl;
	}

	stbi::Image labelImage(bwimage.width(), bwimage.height(), 3);
	labelImage.fill(0);
	srand(100);
	std::cout << "Drawing components" << std::endl;
	for(const auto& component : segs.components())
	{
		// Draw component with random colors:
		uint8_t c0 = static_cast<uint8_t>(255 * ((component[0].label + 1) / static_cast<float>(segs.components().size() + 1)));
		uint8_t c1 = rand() % 255, c2 = rand() % 255;
		if(npixels(component) < 40)
		{
			imtag::draw(component, labelImage.data(), labelImage.width(), labelImage.nchannels(), 0, 0, 0);
			continue;
		}
		imtag::draw(component, labelImage.data(), labelImage.width(), labelImage.nchannels(), c0, c1, c2);

		// Draw bb
		auto bb = imtag::bounding_box<label_t>(component);
		bb.draw(labelImage.data(), labelImage.width(), labelImage.nchannels());
		bb.draw(bwimage.data(), bwimage.width(), bwimage.nchannels());

		// Draw center of component
		auto center = centroid(component);
		if((center.first < labelImage.width()) && (center.second < labelImage.height()))
			labelImage.draw_crosshair(center.first, center.second, 4, 255, 255, 255);
	}
	std::cout << "Writing labels image to labels.png." << std::endl;
	labelImage.write("labels.png");

	return EXIT_SUCCESS;
}
