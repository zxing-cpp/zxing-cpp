cmake_minimum_required (VERSION 3.1.3)

project (ZXingWasm)

set (ENABLE_ENCODERS ON CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

add_definitions (-DUNICODE -D_UNICODE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG ("-std=c++11" COMPILER_SUPPORTS_CXX11)

if (COMPILER_SUPPORTS_CXX11)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

set (SOURCE_FILES lodepng.cpp)

if (ENABLE_DECODERS)
    set (SOURCE_FILES ${SOURCE_FILES}
        BarcodeReader.cpp
    )
endif()

if (ENABLE_ENCODERS)
    set (SOURCE_FILES ${SOURCE_FILES}
        BarcodeWriter.cpp
    )
endif()

add_executable (zxing
    ${SOURCE_FILES}
    lodepng.cpp
)

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --bind -Oz -s DISABLE_EXCEPTION_CATCHING=0 -s FILESYSTEM=0")

target_link_libraries (zxing ZXingCore)
