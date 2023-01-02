#
# This will create a Windows Runtime Component that can be consumed by any WinRT or UWP project.
# You need to specify -DCMAKE_SYSTEM_NAME=WindowsStore -DCMAKE_SYSTEM_VERSION=10.0 at command line
# to generate correctly UWP projects. You can also use Toolchain-Win10.cmake as tool chain file
# which defined these for you.
#
# You can add EXTENSION_SDK_OUTPUT variable to deploy the build as UWP Extension SDK
# e.g. -DEXTENSION_SDK_OUTPUT:STRING=<path_to_folder_parent_of_SDKManifest.xml>
#
cmake_minimum_required (VERSION 3.14)

project (ZXingWinRT)

set (CMAKE_CONFIGURATION_TYPES "RelWithDebInfo;Release" CACHE STRING "" FORCE)
set (WINRT ON)
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO" )

add_definitions (-DUNICODE -D_UNICODE)

if (MSVC)
    add_definitions (
        -DNOMINMAX -wd4996
    )
endif()

include(TargetArch.cmake)
get_target_architecture (TARGET_ARCHITECTURE)
set (TARGET_ARCHITECTURE ${TARGET_ARCHITECTURE} CACHE INTERNAL "")
message (STATUS "Building for " ${TARGET_ARCHITECTURE})

if (NOT DEFINED EXTENSION_SDK_OUTPUT)
    set (EXTENSION_SDK_OUTPUT "" CACHE PATH "Path to folder parent of SDKManifest")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.h)

set (ROOT_NAMESPACE "ZXing")
set (OUTPUT_FILE_BASE_NAME "ZXing")

add_library (ZXingWinRT SHARED ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries (ZXingWinRT
    PRIVATE ZXing::ZXing
)

target_compile_options (ZXingWinRT PRIVATE
    -D_WINRT_DLL
    -ZW             # Consume Windows Runtime Extension
)

set_target_properties (ZXingWinRT PROPERTIES
    LINK_FLAGS "/WINMD"     # Generate Windows Metadata
    VS_GLOBAL_ROOTNAMESPACE "${ROOT_NAMESPACE}"
    OUTPUT_NAME ${OUTPUT_FILE_BASE_NAME}
)

set_target_properties (ZXingWinRT ZXing PROPERTIES
    VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "10.0.10240.0"
)

if (EXTENSION_SDK_OUTPUT)
    add_custom_command (TARGET ZXingWinRT POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "$<TARGET_FILE:ZXingWinRT>" "${EXTENSION_SDK_OUTPUT}/Redist/$<$<CONFIG:Release>:Retail>$<$<CONFIG:RelWithDebInfo>:Debug>/${TARGET_ARCHITECTURE}/$<TARGET_FILE_NAME:ZXingWinRT>"
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "$<TARGET_FILE_DIR:ZXingWinRT>/${ROOT_NAMESPACE}.winmd" "${EXTENSION_SDK_OUTPUT}/References/CommonConfiguration/Neutral/${ROOT_NAMESPACE}.winmd"
        COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "$<TARGET_FILE_DIR:ZXingWinRT>/${ROOT_NAMESPACE}.pri" "${EXTENSION_SDK_OUTPUT}/References/CommonConfiguration/Neutral/${ROOT_NAMESPACE}.pri"
    )
endif()
