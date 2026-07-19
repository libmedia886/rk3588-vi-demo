cmake_minimum_required(VERSION 3.16)
project(rk3588_vi_demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIBMEDIA_SDK "" CACHE PATH "Path to rk3588 libmedia SDK root")
if(NOT LIBMEDIA_SDK)
  message(FATAL_ERROR "Set -DLIBMEDIA_SDK=/path/to/sdk containing include/ and lib/")
endif()

add_executable(rk3588_vi_demo src/main.c)
target_include_directories(rk3588_vi_demo PRIVATE "${LIBMEDIA_SDK}/include")
target_link_directories(rk3588_vi_demo PRIVATE "${LIBMEDIA_SDK}/lib")
target_link_libraries(rk3588_vi_demo PRIVATE media)

