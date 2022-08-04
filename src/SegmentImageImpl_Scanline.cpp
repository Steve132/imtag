#include "SegmentImageImpl.hpp"


//#include "scanline_base.hpp"
//#include "omp_scanline.hpp"
//#include "avx2_scanline.hpp"

#include "scanlines/index.hpp"
#include <thread>

namespace scanline_impl=scanline_base;

// NOTE: This is BY FAR the performance bottleneck:
template<class seg_t>
static void compress_scanline(const uint8_t* bimg,const uint_fast16_t C,std::vector<seg_t>& row_segments, const uint_fast16_t r){
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
		//i++; // optimization - optional, can remove
		i+=scanline_impl::find_next<false>(bimg+i,C-i);
		ending=i;
		row_segments.emplace_back(r,beginning,ending,0);

		// Look for next segment's beginning 1 after end 0
		//i++; // optimization - optional, can remove
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

//#define TRY_THREADED
#ifdef TRY_THREADED
	std::array<std::thread,6> thread_pool;
	uint_fast16_t chunk_size = R16 / thread_pool.size();
	for(uint8_t thread_ind = 0; thread_ind < thread_pool.size(); thread_ind++)
	{
		uint_fast16_t thread_start_r = thread_ind*chunk_size, thread_end_r = std::min(thread_start_r + chunk_size + 1, R16);
		thread_pool[thread_ind] = std::thread(
		[thread_start_r, thread_end_r, &output_rows, binary_image, C16]()
		{
			for(uint_fast16_t r=thread_start_r;r<thread_end_r;r++){
				// Append segments to this scanline
				auto& row=output_rows[r];
				row.clear();
				compress_scanline(binary_image+C16*r,C16,row,r);
			}
		});
	}
	for(uint8_t thread_ind = 0; thread_ind < thread_pool.size(); thread_ind++)
		thread_pool[thread_ind].join();
#else
	// TODO: try force pin image loaded directly to L3 cache
	//#pragma omp parallel for schedule(dynamic)
	//#pragma omp parallel for schedule(static)
	//#pragma omp parallel for num_threads(6)
	#pragma omp parallel for schedule(guided) num_threads(6)
	for(uint_fast16_t r=0;r<R16;r++){
		// Append segments to this scanline
		auto& row=output_rows[r];
		row.clear();
		compress_scanline(binary_image+C16*r,C16,row,r);
	}
#endif

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
