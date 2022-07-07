#include "imtag.hpp"
#include "stb_image_library.h"
#include <chrono>
#include <iostream>

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
	if(argc < 2)
		return EXIT_FAILURE;
	bool do_benchmark = true; //false;
	std::string fname = argv[1];
	std::cout << "Loading: " << fname << std::endl;
	stbi::Image bwimage(fname);

	auto segs = imtag::SegmentImage<uint16_t>(bwimage.height(), bwimage.width());
	if(do_benchmark)
	{
		size_t niters = 2000;
		auto z = [&bwimage, &segs](){ segs.update(bwimage.data()); };
		//auto z = [&bwimage](){ auto segs = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data()); };
		benchmark(z, niters);
	}

	// Debug output:
	//auto segs0 = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data());
	std::cout << "# components: " << segs.components().size() << std::endl;

	for(const auto& component : segs.components())
	{
		auto bb = segs.bounding_box(component);
		bb.draw(bwimage.data(), bwimage.width(), bwimage.nchannels());
	}
	std::cout << "Writing bounding box image to bbs.png." << std::endl;
	bwimage.write("bbs.png");

	return EXIT_SUCCESS;
}
