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

template<class LabelType>
class Segment
{
public:
    using label_t=LabelType;
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

template<class LabelType>
struct Component: public std::vector<Segment<LabelType>>{
    using segment_t=Segment<LabelType>;
    using coord_t=typename Segment<LabelType>::coord_t;
    using label_t=typename Segment<LabelType>::label_t;

    using std::vector<segment_t>::vector;
};

enum class ConnectivitySelection{
    HORIZONTAL,
    //VERTICAL, //this is interesting.
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
    using component_t=Component<label_t>;
    using components_t=std::vector<component_t>;
    
    // Dimensions of source image
    size_t width() const { return m_columns; }
    size_t height() const { return m_rows; }
    size_t rows() const { return m_rows; }
    size_t columns() const { return m_columns; }
    size_t size() const { return m_rows*m_columns; }

	components_t& components();
	const components_t& components() const;

    std::vector<std::vector<segment_t>>& segments_by_row();
    const std::vector<std::vector<segment_t>>& segments_by_row() const;


    SegmentImage(size_t rows,size_t columns);

    // Perform connected components on boolean image
    void update(const uint8_t* boolean_image, const ConnectivitySelection cs = ConnectivitySelection::CROSS);
    void update_connectivity(const ConnectivitySelection cs = ConnectivitySelection::CROSS);

	void to_label_image(label_t* image) const;
	void to_mask_image(uint8_t* image) const;

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
