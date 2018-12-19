cmake_minimum_required (VERSION 3.1.3)

project (ZXingTest)

set (ENABLE_ENCODERS OFF CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

add_definitions (-DUNICODE -D_UNICODE -DLODEPNG_NO_COMPILE_ENCODER)

if (MSVC)
    add_definitions (-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX )

    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-braces")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../../core ${CMAKE_BINARY_DIR}/ZXingCore)

if (ENABLE_DECODERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ../../common/TestReader.h
        ../../common/TestReader.cpp
        ../../common/Pdf417MultipleCodeReader.h
        ../../common/Pdf417MultipleCodeReader.cpp
        ../../common/BlackboxTestRunner.h
        ../../common/BlackboxTestRunner.cpp
        ../../common/lodepng.cpp
    )
    
    target_include_directories (ReaderTest
        PRIVATE ../../common
    )

    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        target_compile_options (ReaderTest PRIVATE -std=c++11)
        target_link_libraries (ReaderTest stdc++fs) # To use std::filesystem
    elseif (APPLE)
        target_compile_options (ReaderTest PRIVATE -std=c++11 -stdlib=libc++)
    endif()

    target_link_libraries (ReaderTest ZXingCore)

endif()

#if (ENABLE_ENCODERS)
#    add_executable (WriterTest
#        TestWriterMain.cpp
#    )
#
#    target_link_libraries (WriterTest ZXingCore)
#endif()
