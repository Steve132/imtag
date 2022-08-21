#include "SegmentImageImpl.hpp"
#include <cstring> // memset
#include <unordered_map>

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
	label_t last_label = components.back().size() ? components.back().back().label : components.size();
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
void SegmentImageImpl<label_t>::to_rgba_adjacencies_image(uint8_t* image, const adjacency_matrix& hole_adjacencies_matrix, const std::array<uint8_t,4>& background_color) const
{
	if(!hole_adjacencies_matrix.columns())
		return;

	std::vector<uint32_t> label_colors32;
	const label_t n_waters = hole_adjacencies_matrix.columns();
	const label_t n_lands = hole_adjacencies_matrix.rows();
	label_colors32.reserve(n_lands);
	std::vector<std::array<uint8_t,3>> water_colors(n_waters);
	srand(100);
	for(label_t water = 0; water < n_waters; water++)
	{
		water_colors[water] = {static_cast<uint8_t>(rand() % 255),static_cast<uint8_t>(rand() % 255),static_cast<uint8_t>(rand() % 255)};
	}

	for(label_t land = 0; land < n_lands; land++)
	{
		// Combine river colors.  Partition rivers into 3 color channels.
		std::array<uint8_t,4> combined_rivers_color = {0,0,0,0xFF};
		for(label_t water = 0; water < n_waters; water++)
		{
			bool connected = hole_adjacencies_matrix(land, water);
			combined_rivers_color[0] |= (water_colors[water][0] * connected);
			combined_rivers_color[1] |= (water_colors[water][1] * connected);
			combined_rivers_color[2] |= (water_colors[water][2] * connected);
		}
		label_colors32.push_back(*reinterpret_cast<const uint32_t*>(combined_rivers_color.data()));
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
	auto& output_rows=out.segments_by_row();

	const uint_fast16_t R16 = static_cast<uint_fast16_t>(src_rows.size());
	#pragma omp parallel for schedule(guided) num_threads(6)
	for(uint_fast16_t r=0;r<R16;r++){
		const auto& src_row=src_rows[r];
		auto& output_row=output_rows[r];
		if(src_row.size()==0) {
			output_row.emplace_back(r,0,a.columns(),0);
			continue;
		}
		uint_fast16_t curcol=0;
		auto cur_seg=src_row.begin();
		if(cur_seg->column_begin==0){
			curcol=cur_seg->column_end;
			if(cur_seg->column_end == a.columns()) continue;
			++cur_seg;
		}
		label_t prev_label = 0;
		for(; cur_seg != src_row.end(); ++cur_seg){
			output_row.emplace_back(r,curcol,cur_seg->column_begin, 1);
			curcol=cur_seg->column_end;
			prev_label=cur_seg->label;
		}
		output_row.emplace_back(r,curcol,a.columns(),0);
	}
	// Assign unique labels across scanlines: linearize labels now that labels assigned per row
	label_t label = 0;
	for(uint_fast16_t r=0;r<R16;r++){
		auto& row_segs=output_rows[r];
		for(auto& seg : row_segs){
			seg.label = label++;
		}
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

/*
Lands:
11110010
11110010
00000010
11111010
	label[0] = [0, 0-4],[1, 1-4]
	label[1] = [0, 6-7],[1, 6-7], [2,6-7], [3,6-7]
	label[2] = [3, 0-5]
and its inverse:
Waters:
00001101
00001101
11111101
00000101
	label[A] = [0,4-6],[1,4-6],[2,0-6],[3,5-6]
	label[B] = [0,7-8],[1,7-8],[2,7-8],[3,7-8]
What are the waters next to an land?
For land label 0, it's water A.
What lands are next to a water?
For water A, lands label 0,1, and 2.
For water B, only land 1.
The adjacency matrix from lands (rows) to waters (columns):
[1 0]
[1 1]
[1 0]
Land 0 is connected to water A and not B.
Land 1 is connected to water A and B.
Land 2 is connected to water A and not B.
*/
template<class label_t>
hole_adjacencies_result<label_t> hole_adjacencies(const SegmentImage<label_t>& input)
{
	const size_t n_lands = input.components().size();
	hole_adjacencies_result<label_t> res = { adjacency_matrix(0,0), invert(input) };
	res.M = adjacency_matrix(n_lands, res.inverted.components().size());

	auto& land_rows = input.segments_by_row();
	auto& water_rows = res.inverted.segments_by_row();
	uint_fast16_t R16 = static_cast<uint_fast16_t>(land_rows.size());
	for(uint_fast16_t ri = 0; ri < R16; ri++) {
		auto& land_row = land_rows[ri];
		//	label[0] = [0, 0-4],[1, 1-4]
		//	label[1] = [0, 6-7],[1, 6-7], [2,6-7], [3,6-7]
		//	label[2] = [3, 0-5]
		//	For scanline row 0 lands: land 0 [0,0-4] and land 1 [0,6-7]
		auto& water_row = water_rows[ri];
		//	label[A] = [0,4-6],[1,4-6],[2,0-6],[3,5-6]
		//	label[B] = [0,7-8],[1,7-8],[2,7-8],[3,7-8]
		//	For scanline row 0 waters: water A [0,4-6] and water B [0,7-8]
		if(!(water_row.size() && land_row.size())) {
			continue; // bail if either row is empty (no adjacencies)
		}

		size_t n_pairs = land_row.size() < water_row.size() ? land_row.size() : water_row.size();
		// Per scanline, connect each pair where a segment connects to the next segment in the inverted image.
		// Because of the definiton of invert, they're guaranteed to be interleaved
		for(size_t i = 0; i < n_pairs; i++) {
			// scanline 0: island 0 [0,0-4] and river A [0,4-6] guaranteed to be connected because immediately next to.  island 1 [0, 6-7] conected to river B [0,7-8].
			// scanline 2: island 1 [2,6-7] and river A [2,0-6]. and skipping river B because no more segments.
			// Howevever, river B is connected to island 1 on the other side.  Set this below.
			res.M(land_row[i].label, water_row[i].label)=true;
		}

		// if the first river segment of this scanline begins before the first island segment:
		if(land_row.front().column_begin > water_row.front().column_begin) {
			for(size_t i = 1; i < n_pairs; i++){
				// scanline 2: island 1 [2,6-7], river B [2,7-8].
				res.M(land_row[i-1].label,water_row[i].label)=true;
			}
		}
		else {
			for(size_t i = 1; i < n_pairs; i++) {
				res.M(land_row[i].label,water_row[i-1].label)=true;
			}
		}
	}
	return res;
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

template class SegmentImage<uint8_t> invert(const SegmentImage<uint8_t>& a);
template class SegmentImage<uint16_t> invert(const SegmentImage<uint16_t>& a);
template class SegmentImage<uint32_t> invert(const SegmentImage<uint32_t>& a);
template class SegmentImage<uint64_t> invert(const SegmentImage<uint64_t>& a);

template hole_adjacencies_result<uint8_t> hole_adjacencies(const SegmentImage<uint8_t>& a);
template hole_adjacencies_result<uint16_t> hole_adjacencies(const SegmentImage<uint16_t>& a);
template hole_adjacencies_result<uint32_t> hole_adjacencies(const SegmentImage<uint32_t>& a);
template hole_adjacencies_result<uint64_t> hole_adjacencies(const SegmentImage<uint64_t>& a);

}
