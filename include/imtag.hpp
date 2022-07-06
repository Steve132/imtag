#ifndef IMTAG_HPP
#define IMTAG_HPP

#include<cstdint>
#include<cstdlib>
#include<memory>
#include<span>

namespace imtag{

#ifdef __cpp_lib_span
template<class T>
using span=std::span<T>;
#else
#include "span_shim.hpp"
#endif

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

enum class ConnectivitySelection{
    HORIZONTAL,
    VERTICAL, //this is interesting.
    CROSS,
    EIGHT_WAY
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
    
    using scanline_t=span<segment_t>;
    using component_t=span<segment_t>;

    using components_t=span<component_t>;
    using scanlines_t=span<scanline_t>;

    // Dimensions of source image
    size_t width() const { return m_columns; }
    size_t height() const { return m_rows; }
    size_t rows() const { return m_rows; }
    size_t columns() const { return m_columns; }
    size_t size() const { return m_rows*m_columns; }

    SegmentImage(size_t rows,size_t columns);

    // Perform connected components on boolean image
    void update(const uint8_t* boolean_image, const ConnectivitySelection cs = CROSS);

    scanlines_t scanlines() const;
    components_t components() const;
    component_t component(const label_t lb) const {
        return components()[lb];
    }
    
    SegmentImage(const SegmentImage&);
    SegmentImage& operator=(const SegmentImage&);
    SegmentImage(SegmentImage&&);
    SegmentImage& operator=(SegmentImage&&);
    ~SegmentImage();
};

template<class label_t>
inline SegmentImage<label_t> bwlabel(const size_t rows,const size_t columns,const uint8_t* boolean_image, const ConnectivitySelection cs = CROSS){
    SegmentImage<label_t> segs(rows,columns);
    segs.update(boolean_image,cs);
    return segs;
}

}

#endif 