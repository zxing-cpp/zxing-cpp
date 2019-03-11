cmake_minimum_required (VERSION 3.1.3)

project (ZXingWasm)

set (ENABLE_ENCODERS ON CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

add_definitions (-DUNICODE -D_UNICODE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra  -Wno-missing-braces")

include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG ("-std=c++11" COMPILER_SUPPORTS_CXX11)

if (COMPILER_SUPPORTS_CXX11)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

if (ENABLE_DECODERS AND ENABLE_ENCODERS)
	add_executable (zxing
        BarcodeReader.cpp
        BarcodeWriter.cpp
		lodepng.cpp
    )
	target_link_libraries (zxing ZXingCore)

elseif (ENABLE_DECODERS)
	add_executable (zxing_reader
        BarcodeReader.cpp
		lodepng.cpp
    )
    target_compile_options (zxing_reader PRIVATE -DLODEPNG_NO_COMPILE_ENCODER)
	target_link_libraries (zxing_reader ZXingCore)

elseif (ENABLE_ENCODERS)
	add_executable (zxing_writer
        BarcodeWriter.cpp
		lodepng.cpp
    )
	target_compile_options (zxing_writer PRIVATE -DLODEPNG_NO_COMPILE_DECODER)
	target_link_libraries (zxing_writer ZXingCore)
endif()

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --bind -Oz -s DISABLE_EXCEPTION_CATCHING=0 -s FILESYSTEM=0 -s MODULARIZE=1 -s EXPORT_NAME=ZXing")
