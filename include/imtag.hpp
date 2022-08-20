#ifndef IMTAG_HPP
#define IMTAG_HPP

#include<cstdint>
#include<cstdlib>
#include<memory>
#include<vector>
#include<functional>
#include<limits>
#include<iostream>

namespace imtag{

template<class LabelType>
class Segment
{
public:
	using label_t=LabelType;
	using coord_t=uint16_t;

	coord_t row;
	coord_t column_begin;
	coord_t column_end;  //one past (exclusive)
	label_t label;
	Segment() : row(0),column_begin(0),column_end(0),label(0) {}
	Segment(const coord_t row_,const coord_t column_begin_, const coord_t column_end_, const label_t label_):
		row(row_),column_begin(column_begin_),column_end(column_end_),label(label_) {}

	friend std::ostream &operator<<(std::ostream &output, const Segment& seg)
	{
		output << "label: " << seg.label << " " << seg.row << ": " << seg.column_begin << ", " << seg.column_end;
		return output;
	}
};

template<class LabelType>
struct Component: public std::vector<Segment<LabelType>>{
	using segment_t=Segment<LabelType>;
	using coord_t=typename Segment<LabelType>::coord_t;
	using label_t=typename Segment<LabelType>::label_t;

	using std::vector<segment_t>::vector;
};

enum class ConnectivitySelection{
	HORIZONTAL,
	//VERTICAL, //this is interesting.
	CROSS,
	EIGHT_WAY
};

//Todo: runtime threading option?
template<class LabelType>
class SegmentImage
{
protected:
	class Impl;
	std::unique_ptr<Impl> impl;
	size_t m_rows;
	size_t m_columns;

public:
	using label_t=LabelType;
	using segment_t=Segment<label_t>;
	using component_t=Component<label_t>;
	using components_t=std::vector<component_t>;

	// Dimensions of source image
	size_t width() const { return m_columns; }
	size_t height() const { return m_rows; }
	size_t rows() const { return m_rows; }
	size_t columns() const { return m_columns; }
	size_t size() const { return m_rows*m_columns; }

	components_t& components();
	const components_t& components() const;

	std::vector<std::vector<segment_t>>& segments_by_row();
	const std::vector<std::vector<segment_t>>& segments_by_row() const;


	SegmentImage(size_t rows,size_t columns);

	// Perform connected components on boolean image
	void update(const uint8_t* boolean_image, const ConnectivitySelection cs = ConnectivitySelection::CROSS);
	void update_connectivity(const ConnectivitySelection cs = ConnectivitySelection::CROSS);

	void to_label_image(label_t* image, const bool inc_labels_for_background_0 = true) const;
	void to_rgba_label_image(uint8_t* image, const std::vector<std::array<uint8_t,4>>& label_colors = {}, const std::array<uint8_t,4>& background_color = {0,0,0,0}) const;
	void to_rgba_adjacencies_image(uint8_t* image, const std::array<uint8_t,4>& background_color = {0,0,0,0}) const;
	void to_mask_image(uint8_t* image) const;

	void remove_components(const std::vector<label_t>& labels);

	SegmentImage(const SegmentImage&);
	SegmentImage& operator=(const SegmentImage&);
	SegmentImage(SegmentImage&&);
	SegmentImage& operator=(SegmentImage&&);
	~SegmentImage();
};

template<class label_t>
inline SegmentImage<label_t> bwlabel(
		const size_t rows,const size_t columns,
		const uint8_t* boolean_image,
		const ConnectivitySelection cs = ConnectivitySelection::CROSS)
{
	SegmentImage<label_t> segs(rows,columns);
	segs.update(boolean_image,cs);
	return segs;
}

// Morphological processing
template<class label_t>
SegmentImage<label_t> invert(const SegmentImage<label_t>& a);

template<class label_t>
SegmentImage<label_t> dilate(const SegmentImage<label_t>& a,int mx,int my);

struct adjacency_matrix
{
private:
	struct write_wrapper
	{
	public:
		write_wrapper(size_t location_, std::vector<bool>& src_) :
			location(location_), src(src_) {}
		write_wrapper& operator=(const bool b) { src[location] = b; return *this; }
		operator bool() { return src[location]; }
	private:
		size_t location;
		std::vector<bool>& src;
	};
public:
	adjacency_matrix(const size_t n_components_, const size_t n_inverted_components_) :
		n_components(n_components_), n_inverted_components(n_inverted_components_), mat(n_components_*n_inverted_components_,0) {}
	bool operator()(const size_t r, const size_t c) const { return mat[r*n_inverted_components + c]; };
	write_wrapper operator()(const size_t r, const size_t c) { return write_wrapper(r*n_inverted_components + c, mat); };
	size_t columns() const { return n_inverted_components; }
	size_t rows() const { return n_components; }
private:
	size_t n_components;
	size_t n_inverted_components;
	std::vector<bool> mat;
};

template<class label_t>
struct hole_adjacencies_result
{
public:
	adjacency_matrix M;
	SegmentImage<label_t> inverted;
};

template<class label_t>
hole_adjacencies_result<label_t> hole_adjacencies(const SegmentImage<label_t>& a);

}

#endif 
