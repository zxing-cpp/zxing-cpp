cmake_minimum_required(VERSION 3.14)
project(ZXingPython)

# get pybind11
include(FetchContent)
set(FETCHCONTENT_QUIET Off)
FetchContent_Declare (pybind11 GIT_REPOSITORY https://github.com/pybind/pybind11.git GIT_TAG v2.5.0)
FetchContent_MakeAvailable (pybind11)

# build the main library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. xzing-cpp EXCLUDE_FROM_ALL)

# build the python module and link the other libs
pybind11_add_module(zxing zxing.cpp)
target_link_libraries(zxing PRIVATE ZXingCore pybind11)
