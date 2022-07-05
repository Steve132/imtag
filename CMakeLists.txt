cmake_minimum_required(VERSION 3.16)

project(imtag VERSION
    0.1.0.1    
)

add_subdirectory(src)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    add_subdirectory(test)
    add_subdirectory(example)
endif()