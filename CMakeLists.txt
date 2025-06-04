cmake_minimum_required(VERSION 3.25.1)

project(fuse_test C)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-O2)

add_executable(fuse_test main.c src/buffer.c src/tg_config.c src/tg_file.c src/tg_storage.c)

target_include_directories(fuse_test PUBLIC "./fuse-3.17.2/include")
target_include_directories(fuse_test PUBLIC "./fuse-3.17.2/build")
target_include_directories(fuse_test PUBLIC "./curl-8.13.0/include/curl")

target_include_directories(fuse_test PUBLIC "./include")

target_link_libraries(fuse_test /usr/lib/x86_64-linux-gnu/libcurl.so.4.8.0)
target_link_libraries(fuse_test "${CMAKE_CURRENT_SOURCE_DIR}/fuse-3.17.2/build/lib/libfuse3.so.3.17.2")
