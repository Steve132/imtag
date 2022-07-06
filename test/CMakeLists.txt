add_subdirectory(deps)
add_executable(imtag_test_blob
    blob.cpp
)
target_link_libraries(imtag_test_blob PRIVATE imtag)

add_executable(imtag_test_cross
	imtag_cross.cpp
)
target_link_libraries(imtag_test_cross PRIVATE imtag stb_image)
