cmake_minimum_required(VERSION 3.25.1)

project(fuse_test C)

add_executable(fuse_test main.c)

target_include_directories(fuse_test PUBLIC "./fuse-3.17.2/include")
target_include_directories(fuse_test PUBLIC "./fuse-3.17.2/build")

target_link_libraries(fuse_test  "${CMAKE_CURRENT_SOURCE_DIR}/fuse-3.17.2/build/lib/libfuse3.so")
