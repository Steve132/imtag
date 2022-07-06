#include "imtag.hpp"
#include "SegmentImageImpl.hpp"
#include <cstring>

namespace imtag{

template<class label_t>
class SegmentImage<label_t>::Impl: public SegmentImageImpl<label_t>{
    using base=SegmentImageImpl<label_t>;
    using base::base;
};

template<class label_t>
SegmentImage<label_t>::SegmentImage(
    size_t rows,size_t columns):m_rows(rows),m_columns(columns),
	impl(std::make_unique<Impl>(rows,columns))
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

template<class label_t>
BoundingBox SegmentImage<label_t>::bounding_box(const typename SegmentImage<label_t>::component_t& component) const
{
	return impl->bounding_box(component);
}

void BoundingBox::draw(uint8_t* image, const size_t image_width, const int nchannels) const
{
	if(right == 0 || right > image_width || right <= left || bottom == 0)
		return;

	// draw top line of bb:
	uint8_t* image_top_left = image + top*image_width*nchannels + left*nchannels;
	memset(image_top_left, 255, (right - left)*nchannels);

	// draw bottom line of bb:
	uint8_t* image_bottom_left = image + bottom*image_width*nchannels + left*nchannels;
	memset(image_bottom_left, 255, (right - left)*nchannels);

	// draw left and right lines of bb:
	for(coord_t y = top; y < bottom; y++)
	{
		// draw left line of bb:
		memset(image + y*image_width*nchannels + left*nchannels, 255, nchannels);
		// draw right line of bb:
		memset(image + y*image_width*nchannels + right*nchannels, 255, nchannels);
	}
}

template class SegmentImage<uint8_t>;
template class SegmentImage<uint16_t>;
template class SegmentImage<uint32_t>;
template class SegmentImage<uint64_t>;


}
