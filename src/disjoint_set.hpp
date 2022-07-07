#ifndef DISJOINT_SET_HPP
#define DISJOINT_SET_HPP

#include<vector>
#include<algorithm>
#include<numeric>
#include<cstdint>
#include<memory>
#include<atomic>

namespace {

struct prand_cmp{
	template<class label_uint_t>
	static bool cmp(label_uint_t ux,label_uint_t uy) {
		ux ^= uy;
		uint64_t v=(uint64_t)ux;
		v*=3499239749UL;
		return (v >> 32) & 0x1;
	}
};
struct lt_cmp{
	template<class label_uint_t>
	static bool cmp(uint32_t ux,uint32_t uy) {
		return ux < uy;
	}
};


//https://arxiv.org/pdf/1911.06347.pdf
template<	class label_uint_t,
			class label_cmp_func=prand_cmp,
			class storage_uint_t=label_uint_t,
			class safe_ref=std::reference_wrapper<storage_uint_t>
		 >
class disjoint_set
{
protected:
	mutable std::vector<storage_uint_t> parents;
public:
	size_t size() const {
		return parents.size();
	}

	disjoint_set(label_uint_t N=0) {
		reset(N);
	}
	void reset(label_uint_t N){
		parents.resize((size_t)N);
		for(size_t i=0;i<N;i++){
			parents[i]=i;
		}
	}

	label_uint_t find(label_uint_t x) const {
		label_uint_t px=safe_ref(parents[x]);
		while(px != x)
		{
			label_uint_t ppx=safe_ref(parents[px]);
			//if(ppx==px) return px; early reject?
			parents[x]=ppx; 	//ON PURPOSE.  This writeback isn't needed for correctness so it can be non-atomic
			x=px;px=ppx;
		}
		return x;
	}
	// freeze labels
	void freeze(label_uint_t* out) const{
		size_t N=size();
		for(size_t i=0;i<N;i++){
			out[i]=find(i);
		}
	}
	// Reduce and linearize labels: need out to be set to label_max before!
	size_t compressed_freeze(label_uint_t* out) const{
		size_t N=size();
		static constexpr label_uint_t UNSET_LABEL = std::numeric_limits<label_uint_t>::max();
		std::fill(out, out + N, UNSET_LABEL);

		// 0 1 2 3 4 5 6 7 8 9
		// a b a a c c d a d b
		// Use out as remap:
		// 0 1 2 3 4 5 6 7 8 9
		// becomes:
		// 0 1 0 0 2 2 3 0 3 1
		label_uint_t new_label = 0;
		for(size_t i=0;i<N;i++){
			label_uint_t r = find(i);
			if(out[r] == UNSET_LABEL)
			{
				out[r] = new_label;
				new_label++;
			}
			out[i] = out[r];
		}
		return new_label;
	}

	// Reduce and linearize labels: not working  TODO: Potentially faster for cache behavior
	size_t compressed_freeze_bitmask(label_uint_t* out) const{
		size_t N=size();
		// 0111...1, where the highest bit is set to 0
		static constexpr label_uint_t HIGH_BIT=static_cast<label_uint_t>(1ULL << (sizeof(label_uint_t)*8-1));

		label_uint_t lsofar=0;
		for(size_t i=0;i<N;i++){
			label_uint_t r=find(i);
			if(r == i) {
				r=lsofar++;

			}
			else{
				r=HIGH_BIT | r;
			}
			out[i]=r;
		}
		for(size_t i=0;i<N;i++){
			label_uint_t r=out[i];
			if(r & HIGH_BIT) {
				out[i]=out[r];
			}
		}
		return lsofar;
	}

	bool unite(label_uint_t x,label_uint_t y) {  //AKA union but that's not a valid keyword in C++
		label_uint_t xroot=find(x);
		label_uint_t yroot=find(y);

		if(xroot==yroot) return false;

		if(label_cmp_func::cmp(xroot,yroot)){
			std::swap(xroot,yroot);
		}
		//(safe_ref(parents[yroot]))=xroot; // doesn't work.
		parents[yroot]=xroot;
		return true;
	}
};

}



#if __cplusplus > 201703L

template<class label_uint_t,class label_cmp_func=prand_cmp>
class parallel_disjoint_set:
		public ::disjoint_set<
			label_uint_t,
			label_uint_t,
			std::atomic_ref<label_uint_t>,
			label_cmp_func
		>
{
protected:
	using base=::disjoint_set<
		label_uint_t,
		label_uint_t,
		std::atomic_ref<label_uint_t>,
		label_cmp_func
	>;

public:
	parallel_disjoint_set(size_t N=0):
		base(N)
	{}
};


#else

template<class label_uint_t,class label_cmp_func=prand_cmp>
class parallel_disjoint_set:
		public ::disjoint_set<
			label_uint_t,
			label_cmp_func,
			std::atomic<label_uint_t>,
			std::reference_wrapper<std::atomic<label_uint_t>>
		>
{
protected:
	using base=::disjoint_set<
		label_uint_t,
		label_cmp_func,
		std::atomic<label_uint_t>,
		std::reference_wrapper<std::atomic<label_uint_t>>
	>;
public:
	parallel_disjoint_set(size_t N=0):
		base(N)
	{}
};

#endif





#endif
