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

	std::string fname="../../test/blobs1_.png";
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

	std::cout << "# components: " << segs.components().size() << std::endl;

	// Invert image:
	auto inverted = invert(segs);
	stbi::Image invertedImage(inverted.columns(), inverted.rows(), 4);
	inverted.to_rgba_label_image(invertedImage.data());
	invertedImage.write("inverted.png");

	if(do_benchmark)
	{
		size_t niters = 500;
		std::cout << "Hole adjacencies benchmark " << std::endl;
		auto z = [&segs](){ hole_adjacencies(segs); };
		benchmark(z, niters);
	}

	stbi::Image adjacenciesImage(segs.columns(), segs.rows(), 4);
	segs.to_rgba_adjacencies_image(adjacenciesImage.data());
	adjacenciesImage.write("adjacencies.png");

	stbi::Image maskImage(bwimage.width(), bwimage.height(), 1);
	benchmark([&maskImage,&segs](){	segs.to_mask_image(maskImage.data()); }, 5000);
	maskImage.write("mask.png");

	std::vector<label_t> labelImage0(bwimage.width() * bwimage.height());
	segs.to_label_image(labelImage0.data(), 0);
	if(segs.components().size() > 1)
	{
		const auto& first_component_segment = segs.components()[1][0];
		const label_t label_image_first_component = (labelImage0.data() + first_component_segment.row * bwimage.width() + first_component_segment.column_begin)[0];
		if(first_component_segment.label != label_image_first_component)
			std::cout << "error with to_label_image." << std::endl;
	}

	// Remove too small/noise components:
	std::vector<label_t> tooSmallLabels;
	for(const auto& component : segs.components())
	{
		if(npixels(component) < 4000)
			tooSmallLabels.push_back(component.front().label);
	}
	segs.remove_components(tooSmallLabels);

	// Draw color label image of components:
	stbi::Image colorLabelImage(bwimage.width(), bwimage.height(), 4);
	segs.to_rgba_label_image(colorLabelImage.data());

	// Draw bounding boxes and centroids per component:
	for(const auto& component : segs.components())
	{
		// Draw bb
		auto bb = imtag::bounding_box<label_t>(component);
		bb.draw(colorLabelImage.data(), colorLabelImage.width(), colorLabelImage.nchannels());

		// Draw center of component
		auto center = centroid(component);
		colorLabelImage.draw_crosshair(center.first, center.second, 4, 255, 255, 255);
	}
	std::cout << "Writing labels image to color_labels.png." << std::endl;
	colorLabelImage.write("color_labels.png");

	return EXIT_SUCCESS;
}
