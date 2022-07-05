#ifndef IMTAG_HPP
#define IMTAG_HPP

#include<cstdint>
#include<cstdlib>
#include<memory>

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

	bool overlap(const Segment& b) const
	{
		return (column_start < b.column_end) && (b.column_start < column_end);
	}
};

struct pixel_connectivity_t{
    uint8_t horizontal : 1;
    uint8_t vertical : 1;
    uint8_t diagonal_up : 1;
    uint8_t diagonal_down : 1;
    uint8_t : 4; //fill to 8 bits
};


//Todo: runtime threading option?
template<class label_t>
class SegmentImage
{
protected:    
    class Impl;
    std::unique_ptr<Impl> impl;
    size_t m_rows;
    size_t m_columns;
public:

    size_t width() const { return m_columns; }
    size_t height() const { return m_rows; }
    size_t rows() const { return m_rows; }
    size_t columns() const { return m_rows; }
    size_t size() const { return m_rows*m_columns; }

    SegmentImage(
        size_t rows,size_t columns,
        pixel_connectivity_t connectivity_checks,
        const pixel_connectivity_t* connectivity_image);
    
    void update(
        const pixel_connectivity_t* connectivity_image);

    SegmentImage(const SegmentImage&);
    SegmentImage& operator=(const SegmentImage&);
    SegmentImage(SegmentImage&&);
    SegmentImage& operator=(SegmentImage&&);
    ~SegmentImage();
};

}

#endif 