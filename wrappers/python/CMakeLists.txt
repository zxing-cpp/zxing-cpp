cmake_minimum_required(VERSION 3.1.0)
project(zxing)

# download and include pybind11
execute_process(COMMAND wget https://github.com/pybind/pybind11/archive/v2.2.4.tar.gz -O ${CMAKE_CURRENT_BINARY_DIR}/pybind11.tar.gz)
execute_process(COMMAND tar -xzf ${CMAKE_CURRENT_BINARY_DIR}/pybind11.tar.gz -C ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/pybind11-2.2.4 EXCLUDE_FROM_ALL)

# build the main library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. xzing-cpp EXCLUDE_FROM_ALL)

# build the python module and link the other libs
pybind11_add_module(zxing zxing.cpp)
target_link_libraries(zxing PRIVATE ZXingCore pybind11)
