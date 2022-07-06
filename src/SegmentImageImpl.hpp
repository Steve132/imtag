#include "imtag.hpp"
#include "disjoint_set.hpp"

namespace imtag{

template<class label_t>
class SegmentImageImpl{
public:
    size_t rows,columns;
    typename SegmentImage<label_t>::components_t components;

    using seg_t=Segment<label_t>;
    disjoint_set<label_t> ds;

    std::vector<
        std::vector<seg_t>
    > segments_by_row;

    std::vector<
        std::vector<seg_t>
    > segments_by_label;

    SegmentImageImpl()=default;
    SegmentImageImpl(size_t rows_,size_t cols_){}

    void update(const uint8_t* binary_image,ConnectivitySelection cs);
    void rows_to_labels();

    template<ConnectivitySelection cs>
    struct cs_tag{};

    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::CROSS>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::HORIZONTAL>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::VERTICAL>);
    void update_compiletime_dispatch(const uint8_t* binary_image,cs_tag<ConnectivitySelection::EIGHT_WAY>);
    
};

}