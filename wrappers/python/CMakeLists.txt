cmake_minimum_required(VERSION 3.14)
project(ZXingPython)

# get pybind11
include(FetchContent)
set(FETCHCONTENT_QUIET Off)
FetchContent_Declare (pybind11 GIT_REPOSITORY https://github.com/pybind/pybind11.git GIT_TAG v2.5.0)
FetchContent_MakeAvailable (pybind11)

# check if we are called from the top-level ZXing project
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
    option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
    option (BUILD_WRITERS "Build with writer support (encoders)" ON)
    option (BUILD_READERS "Build with reader support (decoders)" ON)

    # build the main library
    if (NOT BUILD_SHARED_LIBS)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../core ZXing EXCLUDE_FROM_ALL)

    enable_testing()
endif()

# build the python module
pybind11_add_module(zxing zxing.cpp)
target_link_libraries(zxing PRIVATE ZXing::ZXing)

add_test(NAME PythonTest COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxing>)
