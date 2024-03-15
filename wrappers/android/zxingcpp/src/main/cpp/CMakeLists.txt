cmake_minimum_required(VERSION 3.14)
project(ZXingCppAndroid)

# The zxing-cpp requires at least C++17 to build
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ZXING_READERS ON)
set(ZXING_WRITERS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../core ZXing EXCLUDE_FROM_ALL)

add_library(zxingcpp_android SHARED ZXingCpp.cpp)

target_link_libraries(zxingcpp_android PRIVATE ZXing::ZXing log jnigraphics)

