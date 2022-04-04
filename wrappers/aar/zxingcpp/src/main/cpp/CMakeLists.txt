cmake_minimum_required(VERSION 3.10)
project(ZXingAndroid)

# Force to build by C++17. The zxing-cpp require C++17 to build
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_READERS ON)
set(BUILD_WRITERS ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../core ZXing EXCLUDE_FROM_ALL)

add_library(zxing SHARED ZxingCpp.cpp JNIUtils.cpp)

target_link_libraries(zxing PRIVATE ZXing::ZXing jnigraphics)
