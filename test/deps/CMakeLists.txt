add_library(stb_image
	stb_image.h
	stb_image_write.h
	stb_image_library.h
	stb_image_library.cpp)
target_include_directories(stb_image INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
