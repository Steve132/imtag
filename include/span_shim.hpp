#ifndef SPAN_SHIM_HPP
#define SPAN_SHIM_HPP

template<class T>
class span{
protected:
    T* m_data;
    size_t m_size;
public:
    using element_type=T;
    using value_type=std::remove_cv_t<T>;
    using size_type=size_t;
    using difference_type=std::ptrdiff_t;
    using pointer=T*;
    using reference=T&;
    using iterator=T*;
    using reverse_iterator=std::reverse_iterator<iterator>;
    
    pointer data() const {return m_data;}
    size_t size() const {return m_size;}

    iterator begin() const {return m_data;}
    iterator end(){return m_data+m_size;}
    reverse_iterator rbegin(){return m_data+m_size-1;}
    reverse_iterator rend(){return m_data-1;}
    
    reference operator[](size_type idx) const{return m_data[idx];}
    
    reference front() const {return *begin();}
    reference back() const {return *(m_data+m_size-1);}
    bool empty() const { return static_cast<bool>(m_size);}
};

#endif