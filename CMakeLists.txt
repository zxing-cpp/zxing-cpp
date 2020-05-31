cmake_minimum_required (VERSION 3.10)

project (ZXingCpp VERSION "1.0.8" LANGUAGES CXX)

set (ENABLE_ENCODERS ON CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")
IF (NOT WIN32)
set (BUILD_SHARED_LIBRARY ON CACHE BOOL "Check to build ZXingCore as shared library")
ELSE ()
set (BUILD_SHARED_LIBRARY OFF CACHE BOOL "Check to build ZXingCore as shared library")
ENDIF ()

option (BUILD_EXAMPLES "Build the example barcode reader/writer applicatons" ON)
option (BUILD_BLACKBOX_TESTS "Build the black box reader/writer tests" ON)

if (MSVC)
    set (LINK_CPP_STATICALLY OFF CACHE BOOL "MSVC only, check to link statically standard library (/MT and /MTd)")

    add_definitions (-DUNICODE -D_UNICODE)
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

if (BUILD_SHARED_LIBRARY)
    set (BUILD_SHARED_LIBS ON)
    set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory (core)

if (BUILD_EXAMPLES)
	add_subdirectory (example)
endif()
if (BUILD_BLACKBOX_TESTS)
	add_subdirectory (test/blackbox)
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

configure_package_config_file (
    core/ZXingConfig.cmake.in
    ZXingConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install (
    FILES "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfig.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
