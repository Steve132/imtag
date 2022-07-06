#include "imtag.hpp"
#include "SegmentImageImpl.hpp"

namespace imtag{


template<class label_t>
SegmentImage<label_t>::SegmentImage(
    size_t rows,size_t columns,
    pixel_connectivity_t connectivity_checks):m_rows(rows),m_columns(columns)
{
    //update(connectivity_image);
}
template<class label_t>
void SegmentImage<label_t>::update(
    const pixel_connectivity_t* connectivity_image
){}

template<class label_t>
SegmentImage<label_t>::SegmentImage(const SegmentImage& o):
    impl(std::make_unique<Impl>(*o.impl))
{}
    
template<class label_t>
SegmentImage<label_t>& SegmentImage<label_t>::operator=(const SegmentImage& o){
    impl=std::make_unique<Impl>(*o.impl);
    return *this;
}

template<class label_t>
SegmentImage<label_t>::SegmentImage(SegmentImage&&)=default;
    
template<class label_t>
SegmentImage<label_t>& SegmentImage<label_t>::operator=(SegmentImage&&)=default;

template<class label_t>
SegmentImage<label_t>::~SegmentImage()=default;

template class SegmentImage<int8_t>;
template class SegmentImage<uint8_t>;
template class SegmentImage<int16_t>;
template class SegmentImage<uint16_t>;
template class SegmentImage<int32_t>;
template class SegmentImage<uint32_t>;
template class SegmentImage<int64_t>;
template class SegmentImage<uint64_t>;


}