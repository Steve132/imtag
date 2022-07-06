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
    uint8_t diagonal : 1;
    uint8_t : 5; //fill to 8 bits
};

static constexpr pixel_connectivity_t FOUR_WAY={1,1,0,0};
static constexpr pixel_connectivity_t EIGHT_WAY={1,1,1,1};
static constexpr pixel_connectivity_t HORIZONTAL={1,0,0};

//Todo: runtime threading option?
template<class label_t>
class SegmentImage
{
protected:    
    class Impl;
    std::unique_ptr<Impl> impl;
    size_t m_rows;
    size_t m_columns;
    pixel_connectivity_t* spare_connectivity_buffer();
    
    template<uint8_t pc,class Func>
    void internal_update(Image&& img,Func&& cmpfunc){
        pixel_connectivity_t* cbuf=connectivity_buffer();
        for(size_t r=0;r<(m_rows-1);r++)
        for(size_t c=0;c<(m_columns-1);c++){
            auto p_center=img(r,c);
            pixel_connectivity_t& thpc=m_rows*cbuf+m_columns;
            if constexpr(pc & 0x1){
            {
                auto p_right=img(r,c+1);
                thpc.horizontal=cmpfunc(p_center,p_right);
            }
            if constexpr(pc & 0x2){
                auto p_center=img(r+1,c);
                thpc.vertical=cmpfunc(p_center,p_down);
            }
            auto p2=img(r,c+1);


        }
    }
public:
    // Dimensions of source image
    size_t width() const { return m_columns; }
    size_t height() const { return m_rows; }
    size_t rows() const { return m_rows; }
    size_t columns() const { return m_columns; }
    size_t size() const { return m_rows*m_columns; }

    SegmentImage(
        size_t rows,size_t columns,
        pixel_connectivity_t connectivity_checks);

    void update(const pixel_connectivity_t* connectivity_image);

    template<class Image,class Func>
    void update(Image&& img,Func&& func){
        pixel_connectivity_t* cbuf=connectivity_buffer();
        for(size_t r=0;r<m_rows;r++)
        for(size_t c=0;c<m_columns;c++){
            auto p_center=img(r,c);
            auto p_right=img(r,c+1);
            auto p2=img(r,c+1);


        }
    }

    SegmentImage(const SegmentImage&);
    SegmentImage& operator=(const SegmentImage&);
    SegmentImage(SegmentImage&&);
    SegmentImage& operator=(SegmentImage&&);
    ~SegmentImage();
};

}

#endif 