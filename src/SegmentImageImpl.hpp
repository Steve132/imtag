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
		rows(rows_),columns(cols_),segments_by_row(rows_){}

    void update(const uint8_t* binary_image,ConnectivitySelection cs);
    void update_connectivity(ConnectivitySelection cs);
    void rows_to_components();

	template<ConnectivitySelection cs>
	void update_connectivity();

	void to_label_image(label_t* image) const;
	void to_mask_image(uint8_t* image) const;

    static SegmentImageImpl invert(const SegmentImageImpl& a);
    static SegmentImageImpl dilate(const SegmentImageImpl& a,int mx,int my);
    static SegmentImageImpl label_holes(const SegmentImageImpl& a);
};



}

#endif
