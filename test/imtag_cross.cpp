#include "imtag.hpp"
#include "stb_image_library.h"
#include <iostream>

int main(int argc,char** argv)
{
	if(argc < 2)
		return EXIT_FAILURE;
	std::string fname = argv[1];
	std::cout << "loading: " << fname << std::endl;
	stbi::Image bwimage(fname);
	std::cout << "image dims: " << bwimage.width() << "x" << bwimage.height() << std::endl;
	std::cout << "first value: " << (int)bwimage.data()[0] << std::endl;

	auto segs = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data());
	std::cout << "# components: " << segs.components().size() << std::endl;

	return EXIT_SUCCESS;
}
