#include "imtag.hpp"
#include "disjoint_set.hpp"

namespace imtag{

template<class label_t>
class SegmentImageImpl{
public:
    size_t rows,columns;

    using seg_t=Segment<label_t>;
    disjoint_set<label_t> ds;

    std::vector<
        std::vector<seg_t>
    > segments_by_row;

    std::vector<
        std::vector<seg_t>
    > segments_by_label;


    imtag::span<imtag::span<seg_t>> segments_by_row_spans;
    imtag::span<imtag::span<seg_t>> segments_by_label_spans;

    SegmentImageImpl()=default;
    SegmentImageImpl(size_t rows_,size_t cols_){}

    SegmentImageImpl(const SegmentImageImpl&);
    SegmentImageImpl& operator=(const SegmentImageImpl&);

    void update(const uint8_t* binary_image,ConnectivitySelection cs);
};

}