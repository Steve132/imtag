
add_library(imtag 
    imtag.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag_statistics.hpp
	imtag_statistics.cpp
    disjoint_set.hpp
	SegmentImageImpl.hpp
    SegmentImageImpl.cpp
    SegmentImageImpl_Scanline.cpp
	morphology.cpp

	scanlines/avx2_scanline.hpp
	scanlines/sse4_scanline.hpp
	scanlines/sve_scanline.hpp # if ARMv8.2 or higher
	scanlines/naive_scanline.hpp
	scanlines/scanline_base.hpp
	scanlines/omp_scanline.hpp
	scanlines/neon_scanline.hpp
	scanlines/index.hpp
)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(imtag PRIVATE OpenMP::OpenMP_CXX)
else()
    #target_compile_options(imtag PRIVATE 
    #    $<$<CONFIG:Release>:-Ofast -fopenmp-simd>
	#    $<$<CONFIG:RelWithDebInfo>:-Ofast -fopenmp-simd>
    #)
endif()

target_include_directories(imtag PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_compile_features(imtag INTERFACE cxx_std_11)
target_compile_features(imtag PRIVATE cxx_std_17)

target_compile_options(imtag PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>
	$<$<CONFIG:Debug>: -march=native>)
