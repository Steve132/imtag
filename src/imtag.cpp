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
    const uint8_t* binary_image, const ConnectivitySelection cs)
{
    impl->update(binary_image,cs);
}

template<class label_t>
SegmentImage<label_t>::SegmentImage(const SegmentImage& o):
	impl(std::make_unique<Impl>(*o.impl)),
    m_rows(o.m_rows),
    m_columns(o.m_columns)
{}
    
template<class label_t>
SegmentImage<label_t>& SegmentImage<label_t>::operator=(const SegmentImage& o){
	impl=std::make_unique<Impl>(*o.impl);
    m_rows=o.m_rows;
    m_columns=o.m_columns;
    return *this;
}

template<class label_t>
SegmentImage<label_t>::SegmentImage(SegmentImage&& o):
	impl(std::move(o.impl)),
    m_rows(std::move(o.m_rows)),
    m_columns(std::move(o.m_columns))
{}

template<class label_t>
SegmentImage<label_t>& SegmentImage<label_t>::operator=(SegmentImage&& o){
	impl=std::move(o.impl);
    m_rows=std::move(o.m_rows);
    m_columns=std::move(o.m_columns);
    return *this;
}

template<class label_t>
SegmentImage<label_t>::~SegmentImage()=default;

template<class label_t>
typename SegmentImage<label_t>::components_t& SegmentImage<label_t>::components()
{
	return impl->components;
}

template<class label_t>
const typename SegmentImage<label_t>::components_t& SegmentImage<label_t>::components() const
{
	return impl->components;
}

template class SegmentImage<uint8_t>;
template class SegmentImage<uint16_t>;
template class SegmentImage<uint32_t>;
template class SegmentImage<uint64_t>;


}
