#ifndef SEGMENT_IMAGE_IMPL_HPP
#define SEGMENT_IMAGE_IMPL_HPP

#include "imtag.hpp"
#include "disjoint_set.hpp"

namespace imtag{

template<ConnectivitySelection cs>
struct cs_tag{};

template<class label_t>
class SegmentImageImpl{
public:
    size_t rows,columns;
    typename SegmentImage<label_t>::components_t components;

    using seg_t=Segment<label_t>;

    static void compress_scanlines(const uint8_t* binary_image,size_t R,size_t C,
    std::vector<std::vector<seg_t>>& output_rows);


    disjoint_set<label_t> ds;
    std::vector<label_t> cc_nodes;

    std::vector<
        std::vector<seg_t>
    > segments_by_row;

    SegmentImageImpl()=default;
	SegmentImageImpl(size_t rows_,size_t cols_) :
		rows(rows_),columns(cols_){}

    void update(const uint8_t* binary_image,ConnectivitySelection cs);
    void rows_to_components();

	template<ConnectivitySelection cs>
	void update_compiletime_dispatch_connectivity(const uint8_t* binary_image);

    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::CROSS>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::VERTICAL>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::EIGHT_WAY>);


    
};

}

#endif
