#ifndef DISJOINT_SET_HPP
#define DISJOINT_SET_HPP

#include<vector>
#include<algorithm>
#include<numeric>
#include<cstdint>
#include<memory>
#include<atomic>

namespace {
static inline bool prand_cmp(uint32_t ux,uint32_t uy) {
	ux ^= uy;
	uint64_t v=ux;
	v*=3499239749UL;

	return (v >> 32) & 0x1;
}

//https://arxiv.org/pdf/1911.06347.pdf
template<	class label_uint_t,
			class storage_uint_t,
			class safe_ref
		 >
class disjoint_set_base
{
protected:
	mutable std::vector<storage_uint_t> parents;
public:
	disjoint_set_base(size_t N=0) {
		reset(N);
	}
	void reset(size_t N){
		parents.resize(N);
		std::iota(parents.begin(),parents.end(),0);
	}

	label_uint_t find(label_uint_t x) const {
		label_uint_t px;

		while((px=safe_ref(parents[x])) != x)
		{
			label_uint_t ppx=safe_ref(parents[px]);
			if(ppx == px) return px;
			parents[x]=ppx;  //ON PURPOSE.  This writeback isn't needed for correctness so it can be non-atomic
			x=px;
		}
		return x;
	}

	bool unite(label_uint_t x,label_uint_t y) {  //AKA union but that's not a valid keyword in C++
		label_uint_t xroot=find(x);
		label_uint_t yroot=find(y);

		if(xroot==yroot) return false;

		if(::prand_cmp((uint32_t)xroot,(uint32_t)yroot)){
			std::swap(xroot,yroot);
		}
		(safe_ref(parents[yroot]))=xroot;
		return true;
	}
};

}

template<class label_uint_t>
class disjoint_set:
		public ::disjoint_set_base<
			label_uint_t,
			label_uint_t,
			std::reference_wrapper<label_uint_t>
		>
{
protected:
	using base=::disjoint_set_base<
		label_uint_t,
		label_uint_t,
		std::reference_wrapper<label_uint_t>
	>;
	size_t m_num_groups;

public:
	disjoint_set(size_t N=0):
		base(N),
		m_num_groups(N)
	{}
	bool unite(label_uint_t x, label_uint_t y)
	{
		bool r=base::unite(x,y);
		m_num_groups-=r;
		return r;
	}
	size_t num_groups() const {
		return m_num_groups;
	}
};


#if __cplusplus > 201703L

template<class label_uint_t>
class parallel_disjoint_set:
		public ::disjoint_set_base<
			label_uint_t,
			label_uint_t,
			std::atomic_ref<label_uint_t>
		>
{
protected:
	using base=::disjoint_set_base<
		label_uint_t,
		label_uint_t,
		std::atomic_ref<label_uint_t>
	>;
	size_t m_num_groups;

public:
	parallel_disjoint_set(size_t N=0):
		base(N)
	{}
};


#else

template<class label_uint_t>
class parallel_disjoint_set:
		public ::disjoint_set_base<
			label_uint_t,
			std::atomic<label_uint_t>,
			std::reference_wrapper<std::atomic<label_uint_t>>
		>
{
protected:
	using base=::disjoint_set_base<
		label_uint_t,
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
