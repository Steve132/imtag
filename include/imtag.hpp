#ifndef IMTAG_HPP
#define IMTAG_HPP

#include<cstdint>
#include<cstdlib>

namespace imtag{

template<class label_t>
class Segment
{
public:
    using coord_t=uint32_t;

    coord_t row;
	coord_t column_start;
	coord_t column_end;  //one past (exclusive)
	label_t label;
	Segment() : row(0),column_start(0),column_end(0),label(0) {}
	Segment(const coord_t column_start_, const coord_t column_end_, const label_t label_) : column_start(column_start_),column_end(column_end_),label(label_) {}

	bool overlap(const Segment& b) const
	{
		return (column_start < b.column_end && b.column_start < column_end);
	}
};

class Context{

};

}

#endif 