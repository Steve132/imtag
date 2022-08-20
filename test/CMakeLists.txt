add_subdirectory(deps)
add_subdirectory(cv_comparison)
add_executable(imtag_test_blob
	blob.cpp
)
target_link_libraries(imtag_test_blob PRIVATE imtag)

add_executable(imtag_test
	imtag_test.cpp
)
target_link_libraries(imtag_test PRIVATE imtag stb_image)
target_compile_options(imtag_test PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>)


add_executable(imtag_test_compare_cv
	imtag_test_compare_cv.cpp
)
target_link_libraries(imtag_test_compare_cv PRIVATE imtag stb_image cv_comparison)
target_compile_options(imtag_test_compare_cv PRIVATE
	$<$<CONFIG:Release>:-Ofast -march=native>
	$<$<CONFIG:RelWithDebInfo>:-Ofast -march=native>)
