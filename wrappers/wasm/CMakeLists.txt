cmake_minimum_required (VERSION 3.16)

project (ZXingWasm)

set(CMAKE_CXX_STANDARD 20)

include (${CMAKE_CURRENT_SOURCE_DIR}/../../zxing.cmake)

set    (ZXING_WRITERS "OLD" CACHE STRING "Build with old and/or new writer (encoder) backend (OFF/ON/OLD/NEW)")
option (ZXING_READERS "Build with reader support (decoders)" ON)
option (ZXING_WASM_IMAGE_DECODING "Include readBarcode(s)FromImage, which decodes image files with stb_image" ON)
option (ZXING_WASM_LTO "Build with link time optimization" ON)
set    (ZXING_WASM_EXCEPTIONS "js" CACHE STRING "C++ exception handling: 'js' (emulated via JavaScript) or 'wasm' (native WebAssembly exceptions; requires runtime support)")
set    (ZXING_EMSCRIPTEN_ENVIRONMENT "web" CACHE STRING "Optimize build for given emscripten runtime environment (web/node/shell/worker)")

if (ZXING_WASM_IMAGE_DECODING OR ZXING_WRITERS)
    zxing_add_package_stb()
endif()

if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if (ZXING_WASM_EXCEPTIONS STREQUAL "wasm")
    set (ZXING_WASM_EXCEPTION_FLAGS "-fwasm-exceptions")
elseif (ZXING_WASM_EXCEPTIONS STREQUAL "js")
    set (ZXING_WASM_EXCEPTION_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0")
else()
    message (FATAL_ERROR "ZXING_WASM_EXCEPTIONS must be 'js' or 'wasm'")
endif()
add_compile_options ("SHELL:${ZXING_WASM_EXCEPTION_FLAGS}")

if (ZXING_WASM_LTO)
    add_compile_options (-flto)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXing)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --bind -s ENVIRONMENT=${ZXING_EMSCRIPTEN_ENVIRONMENT} ${ZXING_WASM_EXCEPTION_FLAGS} -s FILESYSTEM=0 -s MODULARIZE=1 -s DYNAMIC_EXECUTION=0 -s EXPORT_NAME=ZXing -s EXPORTED_FUNCTIONS=\"['_malloc', '_free']\" -s EXPORTED_RUNTIME_METHODS=HEAPU8 -s ALLOW_MEMORY_GROWTH=1")

if (ZXING_READERS AND ZXING_WRITERS)
    add_executable (zxing BarcodeReader.cpp BarcodeWriter.cpp)
    target_link_libraries (zxing ZXing::ZXing stb::stb)
    target_compile_definitions (zxing PRIVATE ZXING_WASM_IMAGE_DECODING=$<BOOL:${ZXING_WASM_IMAGE_DECODING}>)
endif()

if (ZXING_READERS)
    configure_file(demo_cam_reader.html demo_cam_reader.html COPYONLY)
    configure_file(demo_reader.html demo_reader.html COPYONLY)
    add_executable (zxing_reader BarcodeReader.cpp)
    target_link_libraries (zxing_reader ZXing::ZXing)
    if (ZXING_WASM_IMAGE_DECODING)
        target_link_libraries (zxing_reader stb::stb)
    endif()
    target_compile_definitions (zxing_reader PRIVATE ZXING_WASM_IMAGE_DECODING=$<BOOL:${ZXING_WASM_IMAGE_DECODING}>)
endif()

if (ZXING_WRITERS)
    configure_file(demo_writer.html demo_writer.html COPYONLY)
    add_executable (zxing_writer BarcodeWriter.cpp )
    target_link_libraries (zxing_writer ZXing::ZXing stb::stb)
endif()

