cmake_minimum_required (VERSION 3.1.3)

project (ZXingTest)

set (ENABLE_ENCODERS ON CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

set (CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_definitions (-DUNICODE -D_UNICODE)
    add_definitions (-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX )

    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

if (ENABLE_DECODERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ImageLoader.h
        ImageLoader.cpp
        Pdf417MultipleCodeReader.h
        Pdf417MultipleCodeReader.cpp
        QRCodeStructuredAppendReader.h
        QRCodeStructuredAppendReader.cpp
        BlackboxTestRunner.h
        BlackboxTestRunner.cpp
    )

    target_include_directories (ReaderTest
        PRIVATE ../../thirdparty/stb
    )

    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        target_link_libraries (ReaderTest stdc++fs) # To use std::experimental::filesystem
    elseif (APPLE)
#        target_compile_options (ReaderTest PRIVATE -stdlib=libc++ -lc++experimental)
    endif()

    target_link_libraries (ReaderTest ZXingCore)

endif()

if (ENABLE_ENCODERS)
	add_executable (WriterTest
		TestWriterMain.cpp
	)

	target_include_directories (WriterTest
		PRIVATE ../../thirdparty/stb
	)

	target_link_libraries (WriterTest ZXingCore)
endif()
