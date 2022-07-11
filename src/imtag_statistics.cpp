#include "imtag_statistics.hpp"
#include <cstring>

namespace imtag{

void BoundingBox::draw(uint8_t* image, const size_t image_width, const int nchannels) const
{
	if(right == 0 || right > image_width || right <= left || bottom == 0)
		return;

	// draw top line of bb:
	uint8_t* image_top_left = image + top*image_width*nchannels + left*nchannels;
	memset(image_top_left, 255, (right - left)*nchannels);

	// draw bottom line of bb:
	uint8_t* image_bottom_left = image + bottom*image_width*nchannels + left*nchannels;
	memset(image_bottom_left, 255, (right - left)*nchannels);

	// draw left and right lines of bb:
	for(coord_t y = top; y < bottom; y++)
	{
		// draw left line of bb:
		memset(image + y*image_width*nchannels + left*nchannels, 255, nchannels);
		// draw right line of bb:
		memset(image + y*image_width*nchannels + right*nchannels, 255, nchannels);
	}
}

}
