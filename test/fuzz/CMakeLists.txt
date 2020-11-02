cmake_minimum_required (VERSION 3.14)

project (ZXingFuzz)

set (CMAKE_CXX clang++)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,fuzzer -march=native")
#set (CMAKE_BUILD_TYPE "RelWithDebInfo")

option (BUILD_WRITERS "Build with writer support (encoders)" ON)
option (BUILD_READERS "Build with reader support (decoders)" ON)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXing)

add_executable (fuzzDataBar fuzzDataBar.cpp)
target_link_libraries (fuzzDataBar ZXing::ZXing)

