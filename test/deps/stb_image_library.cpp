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
	if(nchannels != 0){
		nchannels_=nchannels;
	}
	// copy from stb memory to vectory so that there's no destructor required
	size_t size = width_*height_*nchannels_;
	data_.resize(size / sizeof(uint64_t));
	// ensure data is aligned on uint64_t for align_64 in compress_scanlines:

	memcpy(reinterpret_cast<uint8_t*>(data_.data()), tmp, size);
	stbi_image_free(tmp);
}

Image::Image(const int width, const int height, const int nchannels) :
	width_(width),
	height_(height),
	nchannels_(nchannels == 0 ? 1 : nchannels)
{
	size_t size = width_*height_*nchannels_;
	data_.resize(size / sizeof(uint64_t));
}

void Image::fill(const uint8_t c0)
{
	std::fill(reinterpret_cast<uint8_t*>(data_.data()), reinterpret_cast<uint8_t*>(data_.data() + width_*height_*nchannels_), c0);
}

void Image::write(const std::string& fname) const
{
	std::string extension = ".png";
	if((fname.size() >= 5) && (fname[fname.size() - 4] == '.'))
		extension = fname.substr(fname.size() - 4);

	if(extension == ".png")
	{
		if(stbi_write_png(fname.c_str(), width_, height_, nchannels_, reinterpret_cast<const uint8_t*>(data_.data()), 0) == 0)
			throw std::runtime_error("Could not write png " + fname);
	}
	else if(extension == ".bmp")
	{
		if(stbi_write_bmp(fname.c_str(), width_, height_, nchannels_, reinterpret_cast<const uint8_t*>(data_.data())) == 0)
			throw std::runtime_error("Could not write bmp " + fname);
	}
	else if(extension == ".jpg")
	{
		if(stbi_write_jpg(fname.c_str(), width_, height_, nchannels_, reinterpret_cast<const uint8_t*>(data_.data()), 0) == 0)
			throw std::runtime_error("Could not write jpg " + fname);
	}
	else
		throw std::runtime_error("Unsupported extension requested for write.  Available: png, bmp, jpg.");
}

void Image::draw_point(const uint16_t x, const uint16_t y, const uint8_t c0, const uint8_t c1, const uint8_t c2)
{
	if((x >= width_) || (y >= height_))
		return;
	uint8_t* image_top_left = reinterpret_cast<uint8_t*>(data_.data()) + y*width_*nchannels_ + x*nchannels_;
	image_top_left[0] = c0;
	if(nchannels_ >= 2)
		image_top_left[1] = c1;
	if(nchannels_ >= 3)
		image_top_left[2] = c2;
}

void Image::draw_horz_or_vert_line(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint8_t c0, const uint8_t c1, const uint8_t c2)
{
	if(	(x0 >= width_) || (y0 >= height_) ||
		(x1 >= width_) || (y1 >= height_) )
		return;

	uint8_t* image_top_left = reinterpret_cast<uint8_t*>(data_.data()) + y0*width_*nchannels_ + x0*nchannels_;
	if(y0 == y1)
	{
		if((nchannels_ == 1) || (nchannels_ == 3 && (c0 == c1) && (c0 == c2)) || ((nchannels_ == 2) && (c0 == c1)))
			std::fill(image_top_left, image_top_left + ((x1 - x0)*nchannels_), c0);
		// RGB c0 c1 c2 align struct pixel_t {c0,c1,c2} then std::fill with pixel_t

		uint8_t color[] = {c0, c1, c2};
		for(uint16_t x = 0; x < x1-x0; x++)
		{
			uint8_t* image_pix = (image_top_left + x*nchannels_);
			for(uint16_t c = 0; c < nchannels_; c++)
				image_pix[c] = color[c];
		}
	}
	else if(x0 == x1)
	{
		uint8_t color[] = {c0, c1, c2};
		for(uint16_t y = 0; y < y1-y0; y++)
		{
			uint8_t* image_pix = (image_top_left + y*width_*nchannels_);
			for(uint16_t c = 0; c < nchannels_; c++)
				image_pix[c] = color[c];
		}
	}
	else
	{
		// TODO: y = mx + b from 2 points
	}
}

void Image::draw_crosshair(const uint16_t x, const uint16_t y, const uint16_t half_length, const uint8_t c0, const uint8_t c1, const uint8_t c2)
{
	if((x >= width_) || (y >= height_))
		return;

	// vertical
	draw_horz_or_vert_line(
		x,half_length > y ? 0 : y - half_length,
		x,(std::min)(height_-1,y + half_length + 1), c0, c1, c2);

	// horizontal
	draw_horz_or_vert_line(
		half_length > x ? 0 : x - half_length, y,
		(std::min)(width_-1,x + half_length + 1), y, c0, c1, c2);
}

}

