#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image_library.h"
#include <stdexcept>
#include <iostream> // TODO: REMOVE

namespace stbi
{

Image::Image(const std::string& fname, const int nchannels)
{
	// channels = # 8-bit components per pixel ...
	uint8_t* tmp = stbi_load(fname.c_str(), &width_, &height_, &nchannels_, nchannels);
	if(tmp == nullptr)
	{
		throw std::runtime_error("Could not load " + fname);
	}
	// copy from stb memory to vectory so that there's no destructor required
	size_t size = width_*height_*nchannels_;
	data_.resize(size);
	memcpy(data_.data(), tmp, size);
	stbi_image_free(tmp);
}

Image::Image(const int width, const int height, const int nchannels) :
	width_(width),
	height_(height),
	nchannels_(nchannels == 0 ? 1 : nchannels)
{
	size_t size = width_*height_*nchannels_;
	data_.resize(size);
}

void Image::write(const std::string& fname) const
{
	std::string extension = ".png";
	if((fname.size() >= 5) && (fname[fname.size() - 4] == '.'))
		extension = fname.substr(fname.size() - 4);

	if(extension == ".png")
	{
		if(stbi_write_png(fname.c_str(), width_, height_, nchannels_, data_.data(), 0) == 0)
			throw std::runtime_error("Could not write png " + fname);
	}
	else if(extension == ".bmp")
	{
		if(stbi_write_bmp(fname.c_str(), width_, height_, nchannels_, data_.data()) == 0)
			throw std::runtime_error("Could not write bmp " + fname);
	}
	else if(extension == ".jpg")
	{
		if(stbi_write_jpg(fname.c_str(), width_, height_, nchannels_, data_.data(), 0) == 0)
			throw std::runtime_error("Could not write jpg " + fname);
	}
	else
		throw std::runtime_error("Unsupported extension requested for write.  Available: png, bmp, jpg.");
}

}

