cmake_minimum_required (VERSION 3.10)

project (ZXing VERSION "1.2.0" LANGUAGES CXX)

option (BUILD_WRITERS "Build with writer support (encoders)" ON)
option (BUILD_READERS "Build with reader support (decoders)" ON)
option (BUILD_EXAMPLES "Build the example barcode reader/writer applicatons" ON)
option (BUILD_BLACKBOX_TESTS "Build the black box reader/writer tests" ON)
option (BUILD_UNIT_TESTS "Build the unit tests (don't enable for production builds)" OFF)
option (BUILD_PYTHON_MODULE "Build the python module" OFF)
set(BUILD_SYSTEM_DEPS "AUTO" CACHE STRING "Use system dependencies (AUTO/ALWAYS/NEVER)")

if (WIN32)
    option (BUILD_SHARED_LIBS "Build and link as shared library" OFF)
else()
    option (BUILD_SHARED_LIBS "Build and link as shared library" ON)
endif()

if (MSVC)
    option (LINK_CPP_STATICALLY "MSVC only, link standard library statically (/MT and /MTd)" OFF)

    add_definitions (-DUNICODE -D_UNICODE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
    if (LINK_CPP_STATICALLY)
        set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
    set (DEFAULT_BUILD_TYPE "Release")
    message (STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set (CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if (BUILD_SHARED_LIBS)
    set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)

if (BUILD_UNIT_TESTS)
    add_definitions (-DZXING_BUILD_FOR_TEST)
    if (NOT BUILD_WRITERS OR NOT BUILD_READERS)
        message("Note: To build with unit tests, the library will be build with READERS and WRITERS.")
        set (BUILD_WRITERS ON)
        set (BUILD_READERS ON)
    endif()
endif()

set(BUILD_SYSTEM_DEPS_LIST AUTO ALWAYS NEVER)
set_property(CACHE BUILD_SYSTEM_DEPS PROPERTY STRINGS ${BUILD_SYSTEM_DEPS_LIST})
if(NOT BUILD_SYSTEM_DEPS IN_LIST BUILD_SYSTEM_DEPS_LIST)
    message(FATAL_ERROR "BUILD_SYSTEM_DEPS must be one of ${BUILD_SYSTEM_DEPS_LIST}")
endif()

add_subdirectory (core)

enable_testing()

if (BUILD_EXAMPLES)
    add_subdirectory (example)
endif()
if (BUILD_BLACKBOX_TESTS)
    add_subdirectory (test/blackbox)
endif()
if (BUILD_UNIT_TESTS)
    add_subdirectory (test/unit)
endif()
if (BUILD_PYTHON_MODULE)
    add_subdirectory (wrappers/python)
endif()

set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/ZXing")

install (
    EXPORT ZXingTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE ZXing::
)

install (
    DIRECTORY core/src/
    DESTINATION include/ZXing
    FILES_MATCHING PATTERN "*.h"
)

configure_file (
    core/ZXVersion.h.in
    ZXVersion.h
)

install (
    FILES "${CMAKE_CURRENT_BINARY_DIR}/ZXVersion.h"
    DESTINATION include/ZXing
)

# PC file generation.
if (NOT DEFINED INSTALLDIR)
    set (INSTALLDIR ${CMAKE_INSTALL_PREFIX})
    get_filename_component(INSTALLDIR ${INSTALLDIR} ABSOLUTE)
endif()

IF (NOT WIN32)
    configure_file(zxing.pc.in zxing.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zxing.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
ENDIF()

include (CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file (
    core/ZXingConfig.cmake.in
    ZXingConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install (
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
