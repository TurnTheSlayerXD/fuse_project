cmake_minimum_required(VERSION 3.25.1)

project(socket_test C)

add_executable(socket_test main.c)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-O2)

set(CMAKE_BUILD_TYPE Debug)

target_include_directories(socket_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../curl-8.13.0/include)
target_link_libraries(socket_test /usr/lib/x86_64-linux-gnu/libcurl.so.4.8.0)