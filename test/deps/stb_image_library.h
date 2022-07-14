#ifndef STB_IMAGE_LIBRARY_H
#define STB_IMAGE_LIBRARY_H
#include <string>
#include <vector>
namespace stbi
{

class Image
{
public:
	Image(const int width, const int height, const int nchannels = 0);
	// optional nchannels arg: replace '0' with '1'..'4' to force that many components per pixel
	Image(const std::string& fname, const int nchannels = 0);

	void fill(const uint8_t c0);

	void write(const std::string& fname) const;

	void draw_point(const uint16_t x, const uint16_t y, const uint8_t c0 = 0, const uint8_t c1 = 0, const uint8_t c2 = 0);
	void draw_horz_or_vert_line(const uint16_t x0, const uint16_t y0, const uint16_t x1, const uint16_t y1, const uint8_t c0 = 0, const uint8_t c1 = 0, const uint8_t c2 = 0);
	void draw_crosshair(const uint16_t x, const uint16_t y, const uint16_t half_length = 4, const uint8_t c0 = 0, const uint8_t c1 = 0, const uint8_t c2 = 0);

	int width() const { return width_; }
	int height() const { return height_; }
	int nchannels() const { return nchannels_; }
	const uint8_t* data() const { return data_.data(); }
	uint8_t* data() { return data_.data(); }

private:
	int width_,height_,nchannels_;
	std::vector<uint8_t> data_;
};

}
#endif
