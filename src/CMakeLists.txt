
set(IMTAG_HEADERS 
	${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/../include/imtag_statistics.hpp
)

add_library(imtag 
    imtag.cpp
	${IMTAG_HEADERS}
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

set_property(TARGET imtag PROPERTY
	PUBLIC_HEADER ${IMTAG_HEADERS}
)


find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(imtag PRIVATE OpenMP::OpenMP_CXX)
else()
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    try_compile(OMP_SIMD_FOUND 
        ${CMAKE_CURRENT_BINARY_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/try_openmp_simd.cpp
        COMPILE_DEFINITIONS -fopenmp-simd -D_OPENMP_SIMD 
        LINK_OPTIONS -fopenmp-simd
        CXX_STANDARD 11
    )
    if(OMP_SIMD_FOUND)
        target_compile_options(imtag PRIVATE 
            $<$<CONFIG:Release>:-O3 -fopenmp-simd -D_OPENMP_SIMD>
	    $<$<CONFIG:RelWithDebInfo>:-O3 -fopenmp-simd -D_OPENMP_SIMD>
        )
    endif()
endif()

target_include_directories(imtag PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_compile_features(imtag INTERFACE cxx_std_11)
target_compile_features(imtag PRIVATE cxx_std_17)
if(NOT ANDROID AND NOT IOS)
target_compile_options(imtag PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>
	$<$<CONFIG:Debug>: -march=native>)
else()
if(ANDROID_ABI STREQUAL armeabi-v7a)
    set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS -mfpu=neon)
endif()

endif()
