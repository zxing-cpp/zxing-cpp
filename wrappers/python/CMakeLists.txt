cmake_minimum_required(VERSION 3.11.4)
project(zxing)

# download pybind11
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.2.4
)
FetchContent_GetProperties(pybind11)
if(NOT pybind11_POPULATED)
  FetchContent_Populate(pybind11)
  add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# build the main library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. xzing-cpp EXCLUDE_FROM_ALL)

# build the python module and link the other libs
pybind11_add_module(zxing zxing.cpp)
target_link_libraries(zxing PRIVATE ZXingCore pybind11)
