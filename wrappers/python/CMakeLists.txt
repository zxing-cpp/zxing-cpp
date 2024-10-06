cmake_minimum_required(VERSION 3.18)
project(ZXingPython)

# check if we are called from the top-level ZXing project
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
    # Build with C++20 by default (which enables position independent DataMatrix detection).
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS OFF)
    # Allow the fallback to earlier versions if the compiler does not support it.
    set(CMAKE_CXX_STANDARD_REQUIRED OFF)

    option (BUILD_SHARED_LIBS "Link python module to shared lib" OFF)
    option (ZXING_READERS "Build with reader support (decoders)" ON)
    set    (ZXING_WRITERS "NEW" CACHE STRING "Build with old and/or new writer (encoder) backend (OFF/ON/OLD/NEW)")
    set    (ZXING_DEPENDENCIES "AUTO" CACHE STRING "Fetch from github or use locally installed (AUTO/GITHUB/LOCAL)")
    option (ZXING_EXPERIMENTAL_API "Build with experimental API" ON)
    option (ZXING_USE_BUNDLED_ZINT "Use the bundled libzint for barcode creation/generation" ON)

    set(CORE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/core)
    if(IS_SYMLINK ${CORE_PATH})
        # This is needed because otherwise GCC resolves the symlink which causes paths to randomly
        # be prefixed by /core or by /wrappers/python/core depending on include order.
        set(CORE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../core)
    endif()

    if(EXISTS ${CORE_PATH})
        add_subdirectory(${CORE_PATH} ZXing EXCLUDE_FROM_ALL)
        include(${CMAKE_CURRENT_SOURCE_DIR}/zxing.cmake)
    else()
        message(FATAL_ERROR "Unable to locate zxing source code")
    endif()
endif()

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED) # see https://github.com/pybind/pybind11/issues/4785
set(PYBIND11_FINDPYTHON ON) # see https://github.com/pybind/pybind11/issues/4785
zxing_add_package(pybind11 pybind11 https://github.com/pybind/pybind11.git v2.13.6)

# build the python module
pybind11_add_module(zxingcpp zxing.cpp)
target_link_libraries(zxingcpp PRIVATE ZXing::ZXing)

if (ZXING_READERS AND ZXING_WRITERS)
    add_test(NAME PythonTest COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test.py -v)
    set_property(TEST PythonTest PROPERTY ENVIRONMENT PYTHONPATH=$<TARGET_FILE_DIR:zxingcpp>)
endif()

if (NOT DEFINED ZXING_PYTHON_INSTALL_BINDIR)
    set(ZXING_PYTHON_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
endif()

if (NOT DEFINED ZXING_PYTHON_INSTALL_LIBDIR)
    set(ZXING_PYTHON_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
endif()

install(TARGETS zxingcpp
    COMPONENT python
    RUNTIME DESTINATION "${ZXING_PYTHON_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${ZXING_PYTHON_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${ZXING_PYTHON_INSTALL_LIBDIR}")

