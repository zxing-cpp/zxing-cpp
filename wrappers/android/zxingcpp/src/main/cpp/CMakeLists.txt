cmake_minimum_required(VERSION 3.16)
project(ZXingCppAndroid)

set(ZXING_READERS ON)
set(ZXING_WRITERS OFF)

set(ZXING_EXPERIMENTAL_API ON CACHE BOOL "Enable ZXing experimental API (hardcoded for this wrapper)")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../core ZXing EXCLUDE_FROM_ALL)

add_library(zxingcpp_android SHARED ZXingCpp.cpp)

target_link_options(zxingcpp_android PRIVATE "-Wl,-z,max-page-size=16384")
target_link_libraries(zxingcpp_android PRIVATE ZXing::ZXing log jnigraphics)

