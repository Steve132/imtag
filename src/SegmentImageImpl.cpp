#include "SegmentImageImpl.hpp"


namespace imtag{
    

template<class label_t>
void SegmentImageImpl<label_t>::update(const uint8_t* binary_image,ConnectivitySelection cs){

}

template<class label_t>
SegmentImageImpl<label_t>::SegmentImageImpl(const SegmentImageImpl& o){
    operator=(o);
}

template<class label_t>
SegmentImageImpl<label_t>& SegmentImageImpl<label_t>::operator=(const SegmentImageImpl& o){
    return *this;
}


template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;


}