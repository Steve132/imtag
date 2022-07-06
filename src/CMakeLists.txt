
add_library(imtag 
    imtag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag.hpp
    disjoint_set.hpp
    SegmentImageImpl.cpp
    SegmentImageImpl_Scanline.cpp
)

target_include_directories(imtag PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_compile_features(imtag INTERFACE cxx_std_11)
target_compile_features(imtag PRIVATE cxx_std_17)

target_compile_options(imtag PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>)
