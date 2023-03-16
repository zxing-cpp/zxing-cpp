cmake_minimum_required(VERSION 3.15)
project(ZXingPython)

# Force to build by C++17. The zxing-cpp require C++17 to build
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
option (BUILD_WRITERS "Build with writer support (encoders)" ON)
option (BUILD_READERS "Build with reader support (decoders)" ON)

# build the main library

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core ZXing EXCLUDE_FROM_ALL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../zxing.cmake)
else()
    message(FATAL_ERROR "Unable to locate zxing source code")
endif()

find_package(pybind11 CONFIG)

if (NOT pybind11_FOUND)
    set (pybind11_git_repo https://github.com/pybind/pybind11.git)
    set (pybind11_git_rev v2.10.2)
    zxing_add_package(pybind11 pybind11 ${pybind11_git_repo} ${pybind11_git_rev})
endif()

# build the python module
pybind11_add_module(zxingcpp zxing.cpp)
target_link_libraries(zxingcpp PRIVATE ZXing::ZXing)

if (BUILD_READERS AND BUILD_WRITERS)
    add_test(NAME PythonTest COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxingcpp>)
endif()

install(TARGETS zxingcpp
    COMPONENT python
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
