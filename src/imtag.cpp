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
	impl(std::make_unique<Impl>(rows,columns))
{}
template<class label_t>
void SegmentImage<label_t>::update(
	const uint8_t* binary_image, const ConnectivitySelection cs)
{
	impl->update(binary_image,cs);
}

template<class label_t>
void SegmentImage<label_t>::update_connectivity(const ConnectivitySelection cs)
{
	impl->update_connectivity(cs);
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
typename std::vector<std::vector<Segment<label_t>>>& SegmentImage<label_t>::segments_by_row()
{
	return impl->segments_by_row;
}

template<class label_t>
const typename std::vector<std::vector<Segment<label_t>>>& SegmentImage<label_t>::segments_by_row() const
{
	return impl->segments_by_row;
}


template<class label_t>
void SegmentImage<label_t>::to_label_image(label_t* image, const bool inc_labels_for_background_0) const
{
	impl->to_label_image(image, inc_labels_for_background_0);
}

template<class label_t>
void SegmentImage<label_t>::to_mask_image(uint8_t* image) const
{
	impl->to_mask_image(image);
}

template<class label_t>
void SegmentImage<label_t>::to_rgba_label_image(uint8_t* image, const std::vector<std::array<uint8_t,4>>& label_colors, const std::array<uint8_t,4>& background_color) const
{
	impl->to_rgba_label_image(image, label_colors, background_color);
}

template<class label_t>
void SegmentImage<label_t>::to_rgba_adjacencies_image(uint8_t* image, const std::array<uint8_t,4>& background_color) const
{
	// Find components connected along holes:
	std::vector<std::vector<bool>> M = hole_adjacencies(*this);
	impl->to_rgba_adjacencies_image(image, M, background_color);
}

template<class label_t>
void SegmentImage<label_t>::remove_components(const std::vector<label_t>& labels)
{
	impl->remove_components(labels);
}

template class SegmentImage<uint8_t>;
template class SegmentImage<uint16_t>;
template class SegmentImage<uint32_t>;
template class SegmentImage<uint64_t>;


}
