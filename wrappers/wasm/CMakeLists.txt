cmake_minimum_required (VERSION 3.10)

project (ZXingWasm)

option (ENABLE_ENCODERS "Build encoders" ON)
option (ENABLE_DECODERS "Build decoders" ON)

if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build." FORCE)
endif()

add_definitions ("-s DISABLE_EXCEPTION_CATCHING=0")

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

include_directories ("../../thirdparty/stb")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --bind -s ENVIRONMENT=web -s DISABLE_EXCEPTION_CATCHING=0 -s FILESYSTEM=0 -s MODULARIZE=1 -s EXPORT_NAME=ZXing")

if (ENABLE_DECODERS AND ENABLE_ENCODERS)
	add_executable (zxing BarcodeReader.cpp BarcodeWriter.cpp)
	target_link_libraries (zxing ZXing::Core)
endif()

if (ENABLE_DECODERS)
	add_executable (zxing_reader BarcodeReader.cpp)
	target_link_libraries (zxing_reader ZXing::Core)
endif()

if (ENABLE_ENCODERS)
	add_executable (zxing_writer BarcodeWriter.cpp )
	target_link_libraries (zxing_writer ZXing::Core)
endif()

