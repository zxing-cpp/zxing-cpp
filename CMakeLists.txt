cmake_minimum_required(VERSION 3.16)

project(ZXing)

option (ZXING_READERS "Build with reader support (decoders)" ON)
set    (ZXING_WRITERS "ON" CACHE STRING "Build with old and/or new writer (encoder) backend (OFF/ON/OLD/NEW/BOTH)")
option (ZXING_C_API "Build the C-API" OFF)
option (ZXING_EXPERIMENTAL_API "Build with experimental API" OFF)
option (ZXING_EXAMPLES "Build the example barcode reader/writer applications" ON)
option (ZXING_BLACKBOX_TESTS "Build the black box reader/writer tests" OFF)
option (ZXING_UNIT_TESTS "Build the unit tests (don't enable for production builds)" OFF)
option (ZXING_INSTALL_TEST "Test if the installed/public API works" OFF)
option (ZXING_PYTHON_MODULE "Build the python module" OFF)
set    (ZXING_DEPENDENCIES "AUTO" CACHE STRING "Fetch from github or use locally installed (AUTO/GITHUB/LOCAL)")
option (ZXING_CLANG_TIDY "Enable clang-tidy analysis" OFF)

if (WIN32)
    option (BUILD_SHARED_LIBS "Build and link as shared library" OFF)
else()
    option (BUILD_SHARED_LIBS "Build and link as shared library" ON)
endif()

if (MSVC)
    add_definitions (-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)
    option (ZXING_LINK_CPP_STATICALLY "MSVC only, link standard library statically (/MT and /MTd)" OFF)
    if (LINK_CPP_STATICALLY OR ZXING_LINK_CPP_STATICALLY)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
    set (DEFAULT_BUILD_TYPE "Release")
    message (STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set (CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if (NOT (ZXING_READERS OR ZXING_WRITERS))
    message(FATAL_ERROR "At least one of ZXING_READERS/ZXING_WRITERS must be enabled.")
endif()

set(ZXING_WRITERS_LIST OFF ON OLD NEW BOTH)
set_property(CACHE ZXING_WRITERS PROPERTY STRINGS ${ZXING_WRITERS_LIST})
if(NOT ZXING_WRITERS IN_LIST ZXING_WRITERS_LIST)
    message(FATAL_ERROR "ZXING_WRITERS must be one of ${ZXING_WRITERS_LIST}")
endif()

set(ZXING_DEPENDENCIES_LIST AUTO GITHUB LOCAL)
set_property(CACHE ZXING_DEPENDENCIES PROPERTY STRINGS ${ZXING_DEPENDENCIES_LIST})
if(NOT ZXING_DEPENDENCIES IN_LIST ZXING_DEPENDENCIES_LIST)
    message(FATAL_ERROR "ZXING_DEPENDENCIES must be one of ${ZXING_DEPENDENCIES_LIST}")
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 20)
    set (CMAKE_CXX_STANDARD_REQUIRED ON)
    set (CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CMAKE_COLOR_DIAGNOSTICS ON)

if (ZXING_CLANG_TIDY)
    find_program(CMAKE_CXX_CLANG_TIDY NAMES "clang-tidy" DOC "Path to clang-tidy executable")
    if (NOT CMAKE_CXX_CLANG_TIDY)
        message(WARNING "clang-tidy not found.")
    else()
        message(STATUS "clang-tidy found: ${CMAKE_CXX_CLANG_TIDY}")
        set(ZXING_DISABLE_PCH ON)
    endif()
endif()

# make installed binaries relocatable on macOS and Linux
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
if(APPLE)
    set(CMAKE_MACOSX_RPATH ON) # make the binar
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

add_subdirectory (core)

enable_testing()

include(zxing.cmake)

if (ZXING_EXAMPLES)
    add_subdirectory (example)
endif()
if (ZXING_BLACKBOX_TESTS)
    add_subdirectory (test/blackbox)
endif()
if (ZXING_UNIT_TESTS)
    add_subdirectory (test/unit)
endif()
if (ZXING_INSTALL_TEST)
    add_test(
        NAME InstallTest
        COMMAND
            ${CMAKE_COMMAND}
            -DPROJECT_BINARY_DIR=${CMAKE_BINARY_DIR}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/test/install
            -P ${CMAKE_SOURCE_DIR}/test/install/run.cmake
    )
endif()
if (ZXING_PYTHON_MODULE)
    add_subdirectory (wrappers/python)
endif()
if (ZXING_C_API)
    add_subdirectory (wrappers/c)
endif()
