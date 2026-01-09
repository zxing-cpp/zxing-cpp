cmake_minimum_required (VERSION 3.16)

project (ZXingWasm)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../zxing.cmake)
zxing_add_package_stb()

option (ZXING_WRITERS "Build with writer support (encoders)" OLD)
option (ZXING_READERS "Build with reader support (decoders)" ON)
set    (ZXING_EMSCRIPTEN_ENVIRONMENT "web" CACHE STRING "Optimize build for given emscripten runtime environment (web/node/shell/worker)")

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

add_definitions ("-s DISABLE_EXCEPTION_CATCHING=0")

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXing)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --bind -s ENVIRONMENT=${ZXING_EMSCRIPTEN_ENVIRONMENT} -s DISABLE_EXCEPTION_CATCHING=0 -s FILESYSTEM=0 -s MODULARIZE=1 -s DYNAMIC_EXECUTION=0 -s EXPORT_NAME=ZXing -s EXPORTED_FUNCTIONS=\"['_malloc', '_free']\" -s EXPORTED_RUNTIME_METHODS=HEAPU8 -s ALLOW_MEMORY_GROWTH=1")

if (ZXING_READERS AND ZXING_WRITERS)
    add_executable (zxing BarcodeReader.cpp BarcodeWriter.cpp)
    target_link_libraries (zxing ZXing::ZXing stb::stb)
endif()

if (ZXING_READERS)
    configure_file(demo_cam_reader.html demo_cam_reader.html COPYONLY)
    configure_file(demo_reader.html demo_reader.html COPYONLY)
    add_executable (zxing_reader BarcodeReader.cpp)
    target_link_libraries (zxing_reader ZXing::ZXing stb::stb)
endif()

if (ZXING_WRITERS)
    configure_file(demo_writer.html demo_writer.html COPYONLY)
    add_executable (zxing_writer BarcodeWriter.cpp )
    target_link_libraries (zxing_writer ZXing::ZXing stb::stb)
endif()

