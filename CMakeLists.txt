cmake_minimum_required (VERSION 3.1.3)

set (ZXING_VERSION_MAJOR 1)
set (ZXING_VERSION_MINOR 0)
set (ZXING_VERSION_PATCH 6)

project (ZXingCpp VERSION ${ZXING_VERSION_MAJOR}.${ZXING_VERSION_MINOR}.${ZXING_VERSION_PATCH})

set (ENABLE_ENCODERS ON CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")
set (LINK_CPP_STATICALLY OFF CACHE BOOL "MSVC only, check to link statically standard library (/MT and /MTd)")
set (BUILD_SHARED_LIBRARY OFF CACHE BOOL "Check to build ZXingCore as shared library")

add_definitions (-DUNICODE -D_UNICODE)

if (MSVC)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
    if (LINK_CPP_STATICALLY)
        set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    endif()
else()
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

set (DEFAULT_BUILD_TYPE "Release")

if (NOT CMAKE_BUILD_TYPE)
	message (STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
	set (CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

add_subdirectory (core)

set_target_properties (ZXingCore PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties (ZXingCore PROPERTIES SOVERSION ${ZXING_VERSION_MAJOR})

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
