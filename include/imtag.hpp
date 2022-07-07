#ifndef IMTAG_HPP
#define IMTAG_HPP

#include<cstdint>
#include<cstdlib>
#include<memory>
#include<vector>
#include<functional>
#include<limits>
#include<iostream>

namespace imtag{

template<class label_t>
class Segment
{
public:
    using coord_t=uint16_t;

    coord_t row;
	coord_t column_start;
	coord_t column_end;  //one past (exclusive)
	label_t label;
	Segment() : row(0),column_start(0),column_end(0),label(0) {}
	Segment(const coord_t row_,const coord_t column_start_, const coord_t column_end_, const label_t label_): 
        row(row_),column_start(column_start_),column_end(column_end_),label(label_) {}

	friend std::ostream &operator<<(std::ostream &output, const Segment& seg)
	{
		output << seg.row << ": " << seg.column_start << ", " << seg.column_end;
		return output;
	}
};

enum class ConnectivitySelection{
    HORIZONTAL,
    VERTICAL, //this is interesting.
    CROSS,
    EIGHT_WAY
};

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

//Todo: runtime threading option?
template<class LabelType>
class SegmentImage
{
protected:    
    class Impl;
    std::unique_ptr<Impl> impl;
    size_t m_rows;
    size_t m_columns;
    
public:
    using label_t=LabelType;
    using segment_t=Segment<label_t>;
    using component_t=std::vector<segment_t>;
    using components_t=std::vector<component_t>;
    
    // Dimensions of source image
    size_t width() const { return m_columns; }
    size_t height() const { return m_rows; }
    size_t rows() const { return m_rows; }
    size_t columns() const { return m_columns; }
    size_t size() const { return m_rows*m_columns; }
	components_t& components();
	const components_t& components() const;
	BoundingBox bounding_box(const component_t& component) const;

    SegmentImage(size_t rows,size_t columns);

    // Perform connected components on boolean image
    void update(const uint8_t* boolean_image, const ConnectivitySelection cs = ConnectivitySelection::CROSS);

    SegmentImage(const SegmentImage&);
    SegmentImage& operator=(const SegmentImage&);
    SegmentImage(SegmentImage&&);
    SegmentImage& operator=(SegmentImage&&);
    ~SegmentImage();
};

template<class label_t>
inline SegmentImage<label_t> bwlabel(
        const size_t rows,const size_t columns,
        const uint8_t* boolean_image, 
        const ConnectivitySelection cs = ConnectivitySelection::CROSS)
{
    SegmentImage<label_t> segs(rows,columns);
    segs.update(boolean_image,cs);
    return segs;
}

}

#endif 
