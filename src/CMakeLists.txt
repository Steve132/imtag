
add_library(imtag 
    imtag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag.hpp
    disjoint_set.hpp
	SegmentImageImpl.hpp
    SegmentImageImpl.cpp
    SegmentImageImpl_Scanline.cpp
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	#target_compile_options(imtag PRIVATE "${OpenMP_CXX_FLAGS}")
	target_link_libraries(imtag PUBLIC OpenMP::OpenMP_CXX)
	#target_include_directories(imtag PUBLIC OpenMP::OpenMP_CXX_INCLUDE_DIRS)
endif()

target_include_directories(imtag PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_compile_features(imtag INTERFACE cxx_std_11)
target_compile_features(imtag PRIVATE cxx_std_17)

target_compile_options(imtag PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>)
