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
BoundingBox bounding_box(const std::vector<Segment<LabelType>>& component);

template<class LabelType>
size_t npixels(const std::vector<Segment<LabelType>>& component);

template<class LabelType>
std::pair<typename Segment<LabelType>::coord_t, typename Segment<LabelType>::coord_t> massCenter(const std::vector<Segment<LabelType>>& component);

template<class LabelType>
void draw(const std::vector<Segment<LabelType>>& component, uint8_t* image, const size_t image_width, const int nchannels = 1);

template<class LabelType>
BoundingBox bounding_box(const std::vector<Segment<LabelType>>& component)
{
	BoundingBox bb;
	for(const auto& seg : component)
	{
		if(bb.top > seg.row)
			bb.top = seg.row;
		if(bb.bottom < seg.row)
			bb.bottom = seg.row;
		if(bb.left > seg.column_start)
			bb.left = seg.column_start;
		if(bb.right < seg.column_end)
			bb.right = seg.column_end;
	}
	return bb;
}

template<class LabelType>
size_t npixels(const std::vector<Segment<LabelType>>& component)
{
	size_t sum = 0;
	for(const auto& seg : component)
		sum += seg.column_end - seg.column_start;
	return sum;
}

template <class LabelType>
std::pair<typename Segment<LabelType>::coord_t, typename Segment<LabelType>::coord_t> massCenter(const std::vector<Segment<LabelType>>& component)
{
	if(!component.size())
		return {0,0};

	double a00 = 0, a10 = 0, a01 = 0;
	double x_prev = component[component.size()-1].column_end - 1;
	double y_prev = component[component.size()-1].row;
	double ymax = y_prev, xmax = x_prev, ymin = component[0].row;
	for(const auto& seg : component)
	{
		typename Segment<LabelType>::coord_t y_ = seg.row;
		double y = y_;
		if(seg.column_end - 1 > xmax)
			xmax = seg.column_end - 1;
		for(typename Segment<LabelType>::coord_t x_ = seg.column_start; x_ < seg.column_end; x_++)
		{
			double x = x_;
			double dxy = (x_prev * y) - (x * y_prev);
			a00 += dxy;
			a10 += dxy * (x_prev + x);
			a01 += dxy * (y_prev + y);

			x_prev = x;
			y_prev = y;
		}
	}

	// Center of mass
	double m00 = a00 * 0.5;
	if(std::fabs(m00) > std::numeric_limits<double>::epsilon())
	{
		double m10 = a10 * 0.16666666666666666666666666666667;
		double m01 = a01 * 0.16666666666666666666666666666667;
		if(a00 <= 0)
		{
			m00 *= -1;
			m10 *= -1;
			m01 *= -1;
		}
		double cx = m10 / m00, cy = m01 / m00;
		if((cx >= 0) && (cy >= ymin) && (cy <= ymax) && (cx <= xmax))
			return { static_cast<typename Segment<LabelType>::coord_t>(cx),
					 static_cast<typename Segment<LabelType>::coord_t>(cy)};
	}


	// Default to centroid
	double mean_x = 0, mean_y = 0, n = 0;
	for(const auto& seg : component)
	{
		double y = static_cast<double>(seg.row);
		for(typename Segment<LabelType>::coord_t x = seg.column_start; x < seg.column_end; x++)
			mean_x += static_cast<double>(x);
		mean_y += y * (seg.column_end - seg.column_start);
		n += (seg.column_end - seg.column_start);
	}
	mean_x /= n;
	mean_y /= n;
	return {static_cast<typename Segment<LabelType>::coord_t>(mean_x),
			static_cast<typename Segment<LabelType>::coord_t>(mean_y)};
}

template <class LabelType>
void draw(const std::vector<Segment<LabelType>>& component, uint8_t* image, const size_t image_width, const int nchannels = 1, const uint8_t c0 = 0, const uint8_t c1 = 0, const uint8_t c2 = 0)
{
	for(const auto& seg : component)
	{
		typename Segment<LabelType>::coord_t y = seg.row;
		for(typename Segment<LabelType>::coord_t x = seg.column_start; x < seg.column_end; x++)
		{
			uint8_t* pixel = image + y*image_width*nchannels + x*nchannels;
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
