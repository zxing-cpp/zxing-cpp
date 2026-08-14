cmake_minimum_required(VERSION 3.16)
project(ZXingCppAndroid)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(ZXING_READERS ON)
set(ZXING_WRITERS OFF)

set(ZXING_EXPERIMENTAL_API ON CACHE BOOL "Enable ZXing experimental API (hardcoded for this wrapper)")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../core ZXing EXCLUDE_FROM_ALL)

add_library(zxingcpp_android SHARED ZXingCpp.cpp)

# The visibility presets above only cover code compiled here; symbols coming out of static
# archives (notably libc++_static.a) keep default visibility and are re-exported. Since every
# exported symbol is a --gc-sections root, that pins the whole locale/iostream machinery that
# nothing in this library calls. Only the two JNI entry points need to be visible.
target_link_options(zxingcpp_android PRIVATE "-Wl,-z,max-page-size=16384" "-Wl,--exclude-libs,ALL")
target_link_libraries(zxingcpp_android PRIVATE ZXing::ZXing log jnigraphics)

