#include "imtag.hpp"
#include "imtag_statistics.hpp"
#include "stb_image_library.h"
#include <chrono>
#include <iostream>
#include <cstring>

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

int main(int argc,char** argv)
{

	bool do_benchmark = true; //false;

	std::string fname="../../test/blobs1.png";
	if(argc > 1)
	{
		fname=argv[1];
	}
	std::cout << "Loading: " << fname << std::endl;
	stbi::Image bwimage(fname,1);
	bwimage.write("test.png");

	auto segs = imtag::SegmentImage<uint16_t>(bwimage.height(), bwimage.width());
	if(do_benchmark)
	{
		size_t niters = 2000;
		auto z = [&bwimage, &segs](){ segs.update(bwimage.data(), imtag::ConnectivitySelection::CROSS); };
		//auto z = [&bwimage](){ auto segs = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data()); };
		benchmark(z, niters);
	}

	// Debug output:
	//auto segs0 = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data());
	std::cout << "# components: " << segs.components().size() << std::endl;
	stbi::Image labelImage(bwimage.width(), bwimage.height(), 3);
	labelImage.fill(0);
	srand(100);
	for(const auto& component : segs.components())
	{
		// Draw component with random colors:
		uint8_t c0 = static_cast<uint8_t>(255 * ((component[0].label + 1) / static_cast<float>(segs.components().size() + 1)));
		uint8_t c1 = rand() % 255, c2 = rand() % 255;
		//if(component[0].label == 4)
		imtag::draw(component, labelImage.data(), labelImage.width(), labelImage.nchannels(), c0, c1, c2);

		// Draw bb
		auto bb = imtag::bounding_box<uint16_t>(component);
		bb.draw(labelImage.data(), labelImage.width(), labelImage.nchannels());
		bb.draw(bwimage.data(), bwimage.width(), bwimage.nchannels());

		// Draw center of component
		auto center = massCenter(component);
		if((center.first < labelImage.width()) && (center.second < labelImage.height()))
			labelImage.draw_crosshair(center.first, center.second, 4, 255, 255, 255);
	}
	labelImage.write("labels.png");
	std::cout << "Writing bounding box image to bbs.png." << std::endl;
	bwimage.write("bbs.png");

	return EXIT_SUCCESS;
}
