#include "imtag.hpp"
#include "SegmentImageImpl.hpp"

namespace imtag{

template<class label_t>
class SegmentImage<label_t>::Impl: public SegmentImageImpl<label_t>{
    using base=SegmentImageImpl<label_t>;
    using base::base;
};

template<class label_t>
SegmentImage<label_t>::SegmentImage(
    size_t rows,size_t columns):m_rows(rows),m_columns(columns),
    impl(std::make_unique<Impl>(m_rows,m_columns))
{}
template<class label_t>
void SegmentImage<label_t>::update(
    const uint8_t* binary_image,ConnectivitySelection cs)
{
    impl->update(binary_image,cs);
}

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

template<class label_t>
typename SegmentImage<label_t>::objects_t SegmentImage<label_t>::segments_by_row() const{
    return impl->segments_by_row_spans;
}

template<class label_t>
typename SegmentImage<label_t>::objects_t SegmentImage<label_t>::segments_by_label() const{
    return impl->segments_by_label_spans;
}

template class SegmentImage<uint8_t>;
template class SegmentImage<uint16_t>;
template class SegmentImage<uint32_t>;
template class SegmentImage<uint64_t>;


}