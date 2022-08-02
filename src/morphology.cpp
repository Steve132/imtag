#include "SegmentImageImpl.hpp"

namespace imtag{

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::invert(const SegmentImageImpl& a){}

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::dilate(const SegmentImageImpl& a,int mx,int my){}

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::label_holes(const SegmentImageImpl& a){}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;
}