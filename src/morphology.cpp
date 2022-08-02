#include "SegmentImageImpl.hpp"
#include <cstring> // memset

namespace imtag{

namespace detail
{
template<class pixel_t,class label_t>
void to_label_image(pixel_t* image, const size_t rows, const size_t columns, const std::vector<std::vector<typename SegmentImageImpl<label_t>::seg_t>>& segments_by_row)
{
	// TODO: should this be assumed to be 0 initialized?  Could it be faster?
	std::memset(image, 0, rows * columns * sizeof(pixel_t));

	for(size_t y = 0; y < segments_by_row.size(); y++)
	{
		pixel_t* image_row = image + y*columns;
		const auto& segments = segments_by_row[y];
		for(const auto& segment : segments)
		{
			if constexpr (sizeof(pixel_t) == sizeof(label_t))
				std::fill_n(image_row + segment.column_start, segment.column_end - segment.column_start, segment.label);
			else
				std::fill_n(image_row + segment.column_start, segment.column_end - segment.column_start, 0xFF);
		}
	}
}
}

template<class label_t>
void SegmentImageImpl<label_t>::to_label_image(label_t* image) const
{
	detail::to_label_image<label_t, label_t>(image, rows, columns, segments_by_row);
}

template<class label_t>
void SegmentImageImpl<label_t>::to_mask_image(uint8_t* image) const
{
	detail::to_label_image<uint8_t, label_t>(image, rows, columns, segments_by_row);
}

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::invert(const SegmentImageImpl& a){
    auto& src_rows=a.segments_by_row;
    SegmentImageImpl<label_t> out(a.rows,a.columns);
    auto& dst_rows=out.segments_by_row;

    label_t cur_label=0;
    for(size_t r=0;r<a.rows;r++){
        auto& src_row=src_rows[r];
        auto& dst_row=dst_rows[r];
        if(src_row.size()==0) {
            dst_row.emplace_back(r,0,a.columns,cur_label++);
            continue;
        }
        uint_fast16_t curcol=0;
        auto cur_seg=src_row.begin();
        if(cur_seg->column_start==0){
            curcol=cur_seg->column_end;
            if(cur_seg->column_end==a.columns) continue;
            ++cur_seg;
        }
        for(;cur_seg != src_row.end();++cur_seg){
            dst_row.emplace_back(r,curcol,cur_seg->column_start,cur_label++);
            curcol=cur_seg->column_end;
        }
        if(curcol==a.columns){
            dst_row.emplace_back(r,curcol,a.columns,cur_label++);
        }
    }
    out.update_connectivity<ConnectivitySelection::CROSS>();
    return out;
}

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::dilate(const SegmentImageImpl& a,int mx,int my){

}

template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::label_holes(const SegmentImageImpl& a){
    SegmentImageImpl<label_t> inv_a=SegmentImageImpl<label_t>::invert(a);
    for(size_t r=0;r<a.rows;r++){
        auto& dst_row=inv_a.segments_by_row[r];
        auto& src_row=a.segments_by_row[r];
        dst_row.insert(dst_row.end(),src_row.begin(),src_row.end());
        using seg_t=typename SegmentImageImpl<label_t>::seg_t;
        std::sort(dst_row.begin(),dst_row.end(),[](const seg_t& a,const seg_t& b){
            return a.column_start < b.column_start;
        });
    }
    return inv_a;
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;
}
