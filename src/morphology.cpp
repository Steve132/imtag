#include "SegmentImageImpl.hpp"
#include <cstring> // memset

namespace imtag{

namespace detail
{
template<class pixel_t,class label_t,bool maskmode, bool colormode>
void to_label_image(pixel_t* image, const size_t rows, const size_t columns, const std::vector<std::vector<typename SegmentImageImpl<label_t>::seg_t>>& segments_by_row, const label_t inc_labels_for_background_0 = 1, const std::vector<pixel_t>& label_colors = {}, const pixel_t background_color = 0)
{
	#pragma omp parallel for schedule(dynamic)
	for(size_t y = 0; y < segments_by_row.size(); y++)
	{
		pixel_t* image_row = image + y*columns;
		std::memset(image_row, background_color, columns * sizeof(pixel_t));
		const auto& segments = segments_by_row[y];
		for(const auto& segment : segments)
		{
			if constexpr (maskmode)
				std::fill_n(image_row + segment.column_begin, segment.column_end - segment.column_begin, 0xFF);
			else if constexpr(colormode)
				std::fill_n(image_row + segment.column_begin, segment.column_end - segment.column_begin, label_colors[segment.label]);
			else
				std::fill_n(image_row + segment.column_begin, segment.column_end - segment.column_begin, segment.label + inc_labels_for_background_0);
		}
	}
}
}

template<class label_t>
void SegmentImageImpl<label_t>::to_label_image(label_t* image, const bool inc_labels_for_background_0) const
{
	detail::to_label_image<label_t, label_t,false,false>(image, rows, columns, segments_by_row, inc_labels_for_background_0 ? label_t(1) : label_t(0));
}

template<class label_t>
void SegmentImageImpl<label_t>::to_rgba_label_image(uint8_t* image, const std::vector<std::array<uint8_t,4>>& label_colors, const std::array<uint8_t,4>& background_color) const
{
	std::vector<uint32_t> label_colors32;
	label_t last_label = components.back().back().label;
	label_colors32.reserve(last_label + 1);
	if(label_colors.size())
	{
		std::transform(label_colors.begin(), label_colors.end(), std::back_inserter(label_colors32),
			[](const std::array<uint8_t,4> c) { return *reinterpret_cast<const uint32_t*>(c.data()); });
	}

	// Assign random colors for components with no set label color
	srand(100);
	for(label_t l = 0; l < (last_label + 1 - label_colors.size()); l++)
	{
		std::array<uint8_t,4> color = {uint8_t(rand() % 255), uint8_t(rand() % 255), uint8_t(rand() % 255), 0xFF};
		label_colors32.push_back(*reinterpret_cast<const uint32_t*>(color.data()));
	}

	uint32_t background_color32 = *reinterpret_cast<const uint32_t*>(background_color.data());
	detail::to_label_image<uint32_t, label_t,false,true>(reinterpret_cast<uint32_t*>(image), rows, columns, segments_by_row, label_t(0), label_colors32, background_color32);
}

template<class label_t>
void SegmentImageImpl<label_t>::to_mask_image(uint8_t* image) const
{
	detail::to_label_image<uint8_t, label_t,true,false>(image, rows, columns, segments_by_row);
}

template<class label_t>
SegmentImage<label_t> invert(const SegmentImage<label_t>& a){
	const auto& src_rows=a.segments_by_row();
	SegmentImage<label_t> out(a.rows(),a.columns());
	auto& dst_rows=out.segments_by_row();

	label_t cur_label=0;
	for(size_t r=0;r<a.rows();r++){
		const auto& src_row=src_rows[r];
		auto& dst_row=dst_rows[r];
		if(src_row.size()==0) {
			dst_row.emplace_back(r,0,a.columns(),cur_label++);
			continue;
		}
		uint_fast16_t curcol=0;
		auto cur_seg=src_row.begin();
		if(cur_seg->column_begin==0){
			curcol=cur_seg->column_end;
			if(cur_seg->column_end==a.columns()) continue;
			++cur_seg;
		}
		for(;cur_seg != src_row.end();++cur_seg){
			dst_row.emplace_back(r,curcol,cur_seg->column_begin,cur_label++);
			curcol=cur_seg->column_end;
		}
		dst_row.emplace_back(r,curcol,a.columns(),cur_label++);
	}
	out.update_connectivity(ConnectivitySelection::CROSS);
	return out;
}

template<class label_t>
SegmentImage<label_t> dilate(const SegmentImage<label_t>& a,int mx,int my){
	auto& src_rows=a.segments_by_row();
	SegmentImage<label_t> out(a.rows,a.columns);
	auto& dst_rows=out.segments_by_row();
	for(size_t r=0;r<a.rows;r++){
		auto& src_row=src_rows[r];
		size_t rd_lower=r < my ? 0 : (r-my);
		size_t rd_upper=r+my < a.rows ? r+my : a.rows;
		for(size_t rd=rd_lower; rd < rd_upper; rd++){
			auto& dst_row=dst_rows[rd];
			dst_row.insert(dst_row.end(),src_row.begin(),src_row.end());
		}
	}
	//#pragma omp parallel for
	for(size_t r=0;r<a.rows;r++){
		auto& dst_row=dst_rows[r];
		for(auto& seg : dst_row){
			seg.row=r;
			seg.column_begin=seg.column_begin < mx ? 0 : (seg.column_begin-mx);
			seg.column_end=seg.column_end+mx < a.columns ? seg.column_end+mx : a.columns;
		}
		//TODO rectify row
	}
	//update connectivity
	return out;
}


/*
template<class label_t>
SegmentImageImpl<label_t> SegmentImageImpl<label_t>::label_holes(const SegmentImageImpl& a){
	SegmentImageImpl<label_t> inv_a=SegmentImageImpl<label_t>::invert(a);
	size_t nlabels_orig=a.components.size();
	for(size_t r=0;r<a.rows;r++){
		auto& dst_row=inv_a.segments_by_row[r];
		auto& src_row=a.segments_by_row[r];
		dst_row.insert(dst_row.end(),src_row.begin(),src_row.end());
		using seg_t=typename SegmentImageImpl<label_t>::seg_t;
		std::sort(dst_row.begin(),dst_row.end(),[](const seg_t& a,const seg_t& b){
			return a.column_begin < b.column_begin;
		});
	}
	return inv_a;
}*/

template<class label_t>
void SegmentImageImpl<label_t>::remove_components(const std::vector<label_t>& labels)
{
	static constexpr label_t invalid = std::numeric_limits<label_t>::max();
	std::vector<label_t> new_labels(components.size(), 0);
	for(const label_t& remove_label : labels) {
		new_labels[remove_label] = invalid;
	}
	label_t new_label = 0;
	for(label_t old_label = 0; old_label < new_labels.size(); old_label++) {
		if(new_labels[old_label] != invalid) {
			new_labels[old_label] = new_label;
			new_label++;
		}
	}

	typename SegmentImage<label_t>::components_t components_updated(components.size() - labels.size());
	label_t c = 0;
	for(auto& component : components) {
		label_t& update_label = new_labels[component.front().label];
		if(update_label != invalid) {
			components_updated[c] = std::move(component);
			for(auto& seg : components_updated[c]) {
				seg.label = update_label;
			}
			c++;
		}
	}
	std::swap(components,components_updated);

	for(auto& scanline : segments_by_row) {
		typename std::vector<SegmentImageImpl<label_t>::seg_t> scanline_updated;
		scanline_updated.reserve(scanline.size());
		for(auto& seg : scanline) {
			label_t& update_label = new_labels[seg.label];
			if(update_label != invalid) {
				seg.label = update_label;
				scanline_updated.emplace_back(std::move(seg));
			}
		}
		std::swap(scanline, scanline_updated);
	}
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

template class SegmentImage<uint8_t> invert(const SegmentImage<uint8_t>& a);
template class SegmentImage<uint16_t> invert(const SegmentImage<uint16_t>& a);
template class SegmentImage<uint32_t> invert(const SegmentImage<uint32_t>& a);
template class SegmentImage<uint64_t> invert(const SegmentImage<uint64_t>& a);

}
