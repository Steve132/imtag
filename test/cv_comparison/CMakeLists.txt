add_library(cv_comparison
	cvConnectedComponentsWithStats.h
	cvConnectedComponentsWithStats.cpp
)

target_compile_features(cv_comparison PRIVATE cxx_std_17)

target_include_directories(cv_comparison INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_options(cv_comparison PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>)

# OpenCV:
find_package(OpenCV)
if(OpenCV_FOUND)
  target_include_directories(cv_comparison PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_directories(cv_comparison INTERFACE ${OpenCV_LIB_DIR})
  target_link_libraries(cv_comparison INTERFACE ${OpenCV_LIBS})
endif()

