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
void SegmentImageImpl<label_t>::to_rgba_adjacencies_image(uint8_t* image, const std::vector<std::vector<bool>>& hole_adjacencies_matrix, const std::array<uint8_t,4>& background_color) const
{
	if(!hole_adjacencies_matrix.size())
		return;

	std::vector<uint32_t> label_colors32;
	const size_t n_islands = hole_adjacencies_matrix.size();
	label_colors32.reserve(n_islands);
	const size_t n_rivers = hole_adjacencies_matrix[0].size();
	const size_t one_third_n_rivers = n_rivers / 3;
	std::vector<uint8_t> river_colors(n_rivers,0);
	srand(100);
	for(size_t river = 0; river < n_rivers; river++)
		river_colors[river] = rand() % 255;
	for(label_t island = 0; island < n_islands; island++)
	{
		const auto& island_adjacencies = hole_adjacencies_matrix[island];
		// Combine river colors.  Partition rivers into 3 color channels.
		std::array<uint8_t,4> combined_rivers_color;
		for(size_t channel = 0; channel < 3; channel++)
		{
			uint8_t channel_color = 0;
			const size_t rivers_channel_end = one_third_n_rivers * (channel + 1);
			for(size_t river = one_third_n_rivers * channel; river < rivers_channel_end; river++)
				channel_color |= (river_colors[river] * island_adjacencies[river]);
			combined_rivers_color[channel] = channel_color;
		}
		combined_rivers_color[3] = 0xFF;
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

/*
Islands:
11110010
11110010
00000010
11111010
	label[0] = [0, 0-4],[1, 1-4]
	label[1] = [0, 6-7],[1, 6-7], [2,6-7], [3,6-7]
	label[2] = [3, 0-5]
and its inverse:
Rivers:
00001101
00001101
11111101
00000101
	label[A] = [0,4-6],[1,4-6],[2,0-6],[3,5-6]
	label[B] = [0,7-8],[1,7-8],[2,7-8],[3,7-8]
What are the rivers next to an island?
For island label 0, it's river A.
What islands are next to a river?
For river A, islands label 0,1, and 2.
For river B, only island 1.
The adjacency matrix from islands (rows) to rivers (columns):
[1 0]
[1 1]
[1 0]
Island 0 is connected to river A and not B.
Island 1 is connected to river A and B.
Island 2 is connected to river A and not B.
*/
template<class label_t>
std::vector<std::vector<bool>> hole_adjacencies(const SegmentImage<label_t>& input)
	//, SegmentImage<label_t>& inverted)
{
	auto inverted = invert(input);
	const size_t n_islands = input.components().size();
	const size_t n_rivers = inverted.components().size();

	//adj[A][B]=1 iff comp(A)->hole(B)
	//todo should this be an adjacency list? No.
	//normally don't use vector bool.  This time it might be okay because cache.
	std::vector<std::vector<bool>> adj_matrix(n_islands, std::vector<bool>(n_rivers, false));

	auto& island_rows = input.segments_by_row();
	auto& river_rows = inverted.segments_by_row();
	const size_t R = island_rows.size();

	// #pragma omp parallel for schedule(dynamic)
	for(size_t ri = 0; ri < R; ri++) {
		auto& island_row = island_rows[ri];
		//	label[0] = [0, 0-4],[1, 1-4]
		//	label[1] = [0, 6-7],[1, 6-7], [2,6-7], [3,6-7]
		//	label[2] = [3, 0-5]
		//	For scanline row 0 islands: island 0 [0,0-4] and island 1 [0,6-7]
		auto& river_row = river_rows[ri];
		//	label[A] = [0,4-6],[1,4-6],[2,0-6],[3,5-6]
		//	label[B] = [0,7-8],[1,7-8],[2,7-8],[3,7-8]
		//	For scanline row 0 rivers: river A [0,4-6] and river B [0,7-8]
		if(!(river_row.size() && island_row.size())) {
			continue; // bail if either row is empty (no adjacencies)
		}

		size_t n_pairs = island_row.size() < river_row.size() ? island_row.size() : river_row.size();
		// Per scanline, connect each pair where a segment connects to the next segment in the inverted image.
		// Because of the definiton of invert, they're guaranteed to be interleaved
		for(size_t i = 0; i < n_pairs; i++) {
			// scanline 0: island 0 [0,0-4] and river A [0,4-6] guaranteed to be connected because immediately next to.  island 1 [0, 6-7] conected to river B [0,7-8].
			// scanline 2: island 1 [2,6-7] and river A [2,0-6]. and skipping river B because no more segments.
			// Howevever, river B is connected to island 1 on the other side.  Set this below.
			adj_matrix[island_row[i].label][river_row[i].label]=true;
		}

		// No further adjacencies to find:
		if(island_row.size() == river_row.size())
			continue;

		// if the first river segment of this scanline begins before the first island segment:
		if(island_row.front().column_begin > river_row.front().column_begin) {
			for(size_t i = 1; i < n_pairs; i++){
				// scanline 2: island 1 [2,6-7], river B [2,7-8].
				adj_matrix[island_row[i-1].label][river_row[i].label]=true;
			}
		}
		else {
			for(size_t i = 1; i < n_pairs; i++) {
				adj_matrix[island_row[i].label][river_row[i-1].label]=true;
			}
		}
	}
	return adj_matrix;
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

template class SegmentImage<uint8_t> invert(const SegmentImage<uint8_t>& a);
template class SegmentImage<uint16_t> invert(const SegmentImage<uint16_t>& a);
template class SegmentImage<uint32_t> invert(const SegmentImage<uint32_t>& a);
template class SegmentImage<uint64_t> invert(const SegmentImage<uint64_t>& a);

template class std::vector<std::vector<bool>> hole_adjacencies(const SegmentImage<uint8_t>& a);
template class std::vector<std::vector<bool>> hole_adjacencies(const SegmentImage<uint16_t>& a);
template class std::vector<std::vector<bool>> hole_adjacencies(const SegmentImage<uint32_t>& a);
template class std::vector<std::vector<bool>> hole_adjacencies(const SegmentImage<uint64_t>& a);
}
