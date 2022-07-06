#include "imtag.hpp"
#include "stb_image_library.h"
#include <iostream>

int main(int argc,char** argv)
{
	if(argc < 2)
		return EXIT_FAILURE;
	std::string fname = argv[1];
	std::cout << "Loading: " << fname << std::endl;
	stbi::Image bwimage(fname);

	auto segs = imtag::bwlabel<uint16_t>(bwimage.height(), bwimage.width(), bwimage.data());
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
