#include "SegmentImageImpl.hpp"

namespace avx2{
#include "avx2_scanline.hpp"
}
bool check64(const std::array<uint8_t,64>& a)
{
    uint8_t r=0;
    #pragma omp simd reduction(|:r)
    for (size_t i = 0 ; i < 64 ; i++) {
        r |= a[i];
    }
    return r!=0;
}
namespace naive{
    template<bool mask>
    uint_fast16_t find_next(const uint8_t* bimg,uint_fast16_t N){
        for(uint_fast16_t i=0;i<N;i++){
            static constexpr uint8_t TEST= mask ? 0xFF : 0x00;
            if(bimg[i]==TEST){
                return i;
            }
        }
        return N;
    }
}

namespace naive_align64{
	template<size_t W>
	static uint_fast16_t dynamic_alignof(const uint8_t* bimg){
		return static_cast<uint_fast16_t>(reinterpret_cast<uintptr_t>(bimg) & (W-1));
	}
    template<bool mask>
    uint_fast16_t find_next(const uint8_t* bimg,uint_fast16_t N){
		uint_fast16_t offset=0;
		uint_fast16_t a1=dynamic_alignof<8>(bimg);
		uint_fast16_t Nupper=8-a1;
		offset= (a1 == 0) ? Nupper : naive::find_next<mask>(bimg,Nupper);
		if(offset < Nupper) return offset;
		N-=Nupper;
		const uint64_t* bimg64=reinterpret_cast<const uint64_t*>(bimg+Nupper);
		uint_fast16_t M=N/8;
		for(uint_fast16_t i=0;i<M;i++){
            static constexpr uint64_t TEST64= (mask ? 0x0ULL : 0xFFFFFFFFFFFFFFFFULL);
			if(bimg64[i]!=TEST64){
                return offset+naive::find_next<mask>(bimg+offset+8*i,8);
            }
        }
		offset+=M*8;
		if(offset >= N) return N;
		return offset+naive::find_next<mask>(bimg+offset,N-offset);
    }
}

// NOTE: This is BY FAR the performance bottleneck:
template<class MakeSeg>
static void compress_scanline(const uint8_t* bimg,const uint_fast16_t rindex,const uint_fast16_t C,MakeSeg&& msfunc){
	uint_fast16_t i=0;
	// Iterate over one row
    while(i<C){
		// Search for 1s
		uint_fast16_t beginning;
        i+=naive::find_next<true>(bimg+i,C-i);
        if(i==C) {
            break;
        }
        beginning=i;
		uint_fast16_t ending=C;
        i+=naive::find_next<false>(bimg+i,C-i);
		ending=i;
        msfunc(rindex,beginning,ending);
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
	#pragma omp parallel for
	for(uint_fast16_t r=0;r<R16;r++){
		// Append segments to this scanline
		label_t rlabel = 0;
		auto& rows=output_rows[r];
		rows.clear();
		compress_scanline(binary_image+C16*r,r,C16,
			// Make segment (seg_t) function:
			[&rows,&rlabel](uint_fast16_t rind,uint_fast16_t cbegin,uint_fast16_t cend){
				rows.emplace_back(rind,cbegin,cend,rlabel++);
			}
		);
	}

	// Assign unique labels across scanlines: linearize labels now that labels assigned per row
	label_t label = 0;
	for(size_t r=0;r<R;r++){
		auto& rows=output_rows[r];
		for(auto& row : rows)
		{
			row.label = label++;
		}
	}
}

template class SegmentImageImpl<uint8_t>;
template class SegmentImageImpl<uint16_t>;
template class SegmentImageImpl<uint32_t>;
template class SegmentImageImpl<uint64_t>;

}
