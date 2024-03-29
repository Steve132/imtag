#ifndef IMTAG_STATISTICS_HPP
#define IMTAG_STATISTICS_HPP
#include "imtag.hpp"
#include <cmath>

namespace imtag{

class BoundingBox
{
public:
	using coord_t=uint16_t;

	BoundingBox(const coord_t top_ = std::numeric_limits<coord_t>::max(), const coord_t left_ = std::numeric_limits<coord_t>::max(), const coord_t right_ = 0, const coord_t bottom_ = 0) :
		top(top_), left(left_), right(right_), bottom(bottom_)
	{}
	void draw(uint8_t* image, const size_t image_width, const int nchannels = 1) const;
	friend std::ostream &operator<<(std::ostream &output, const BoundingBox& bb)
	{
		output << bb.left << ", " << bb.top << " to " << bb.right << ", " << bb.bottom;
		return output;
	}
	coord_t top;	// y coord
	coord_t left;	// x coord
	coord_t right;	// x coord (just past bb)
	coord_t bottom;	// y coord (just past bb)
};

// TODO: Make Component a type, add these methods to component:
template<class LabelType>
BoundingBox bounding_box(const Component<LabelType>& component);

template<class LabelType>
size_t npixels(const Component<LabelType>& component);

template<class LabelType>
std::pair<typename Segment<LabelType>::coord_t, typename Segment<LabelType>::coord_t> massCenter(const Component<LabelType>& component);

template<class LabelType>
void draw(const Component<LabelType>& component, uint8_t* image, const size_t image_width, const int nchannels = 1);

template<class LabelType>
BoundingBox bounding_box(const Component<LabelType>& component)
{
	BoundingBox bb;
	if(!component.size())
		return bb;
	bb.top = component[0].row;
	bb.bottom = component[component.size() - 1].row + 1;
	for(const auto& seg : component)
	{
		if(seg.column_begin < bb.left)
			bb.left = seg.column_begin;
		if(seg.column_end > bb.right)
			bb.right = seg.column_end;
	}
	return bb;
}

template<class LabelType>
size_t npixels(const Component<LabelType>& component)
{
	size_t sum = 0;
	for(const auto& seg : component)
		sum += seg.column_end - seg.column_begin;
	return sum;
}

template <class LabelType>
std::pair<typename Segment<LabelType>::coord_t, typename Segment<LabelType>::coord_t> 
	centroid(const Component<LabelType>& component)
{
	size_t sx=0,sy=0;
	size_t t=0;

	for(const Segment<LabelType>& seg : component){
		// Closed form solution for sequence of for(x = col start to col end) tx += x:
		size_t tx=((seg.column_end-1)*seg.column_end) - ((seg.column_begin-1)*seg.column_begin);
		size_t n=seg.column_end-seg.column_begin;
		sx+=tx;
		sy+=n*seg.row;
		t+=n;
	}
	return std::make_pair<typename Segment<LabelType>::coord_t,typename Segment<LabelType>::coord_t>(
		sx/(2*t),sy/t
	);
}

template <class LabelType>
void draw(const Component<LabelType>& component, uint8_t* image, const size_t image_width, const int nchannels = 1, const uint8_t c0 = 0, const uint8_t c1 = 0, const uint8_t c2 = 0)
{
	for(const auto& seg : component)
	{
		typename Segment<LabelType>::coord_t y = seg.row;
		uint8_t* image_row = image + y*image_width*nchannels;
		for(typename Segment<LabelType>::coord_t x = seg.column_begin; x < seg.column_end; x++)
		{
			uint8_t* pixel = image_row + x*nchannels;
			pixel[0] = c0;
			if(nchannels >= 2)
				pixel[1] = c1;
			if(nchannels >= 3)
				pixel[2] = c2;
		}
	}
}


}

#endif
