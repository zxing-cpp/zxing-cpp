cmake_minimum_required(VERSION 3.15)
project(ZXingPython)

set (pybind11_git_repo https://github.com/pybind/pybind11.git)
set (pybind11_git_rev v2.11.1)

# check if we are called from the top-level ZXing project
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
    # Build with C++20 by default (which enables position independent DataMatrix detection).
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS OFF)
    # Allow the fallback to earlier versions if the compiler does not support it.
    set(CMAKE_CXX_STANDARD_REQUIRED OFF)

    option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
    option (BUILD_WRITERS "Build with writer support (encoders)" ON)
    option (BUILD_READERS "Build with reader support (decoders)" ON)
    set(BUILD_DEPENDENCIES "AUTO")
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/core)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core ZXing EXCLUDE_FROM_ALL)
        include(${CMAKE_CURRENT_SOURCE_DIR}/zxing.cmake)
    else()
        message(FATAL_ERROR "Unable to locate zxing source code")
    endif()
endif()

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED) # see https://github.com/pybind/pybind11/issues/4785
zxing_add_package(pybind11 pybind11 ${pybind11_git_repo} ${pybind11_git_rev})

# build the python module
pybind11_add_module(zxingcpp zxing.cpp)
target_link_libraries(zxingcpp PRIVATE ZXing::ZXing)

if (BUILD_READERS AND BUILD_WRITERS)
    add_test(NAME PythonTest COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxingcpp>)
endif()

install(TARGETS zxingcpp
    COMPONENT python
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")

