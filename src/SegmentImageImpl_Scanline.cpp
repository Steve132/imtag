#include "SegmentImageImpl.hpp"


//#include "scanline_base.hpp"
//#include "omp_scanline.hpp"
//#include "avx2_scanline.hpp"

#include "scanlines/index.hpp"

namespace scanline_impl=scanline_base;

// NOTE: This is BY FAR the performance bottleneck:
template<class MakeSeg>
static void compress_scanline(const uint8_t* bimg,const uint_fast16_t C,MakeSeg&& msfunc){
	uint_fast16_t i=0;
	// Iterate over one row
    while(i<C){
		// Search for 1s
		uint_fast16_t beginning;
		i+=scanline_impl::find_next<true>(bimg+i,C-i);
		if(i==C) {
            break;
        }
        beginning=i;
		uint_fast16_t ending=C;
		// Look for end (first 0 after beginning)
		i++; // optimization - optional, can remove
		i+=scanline_impl::find_next<false>(bimg+i,C-i);
		ending=i;
		msfunc(beginning,ending);

		// Look for next segment's beginning 1 after end 0
		i++; // optimization - optional, can remove
    }
}


namespace imtag{

template<class label_t>
void SegmentImageImpl<label_t>::compress_scanlines(
    const uint8_t* binary_image,
    size_t R,size_t C,
    std::vector<std::vector<seg_t>>& output_rows){
	output_rows.resize(R);

	uint_fast16_t R16 = static_cast<uint_fast16_t>(R);
	uint_fast16_t C16 = static_cast<uint_fast16_t>(C);
	//#pragma omp parallel for
	for(uint_fast16_t r=0;r<R16;r++){
		// Append segments to this scanline
		auto& rows=output_rows[r];
		rows.clear();
		compress_scanline(binary_image+C16*r,C16,
			// Make segment (seg_t) function:
			[&rows,&r](const uint_fast16_t cbegin,const uint_fast16_t cend){
				rows.emplace_back(r,cbegin,cend,0);
			}
		);
	}

	// Assign unique labels across scanlines: linearize labels now that labels assigned per row
	label_t label = 0;
	for(uint_fast16_t r=0;r<R16;r++){
		auto& rows=output_rows[r];
		for(auto& row : rows){
			row.label = label++;
		}
	}
}


template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}
