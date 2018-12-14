cmake_minimum_required (VERSION 2.8.9)

project (ZXingTest)

set (ENABLE_ENCODERS OFF CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

add_definitions (-DUNICODE -D_UNICODE)

if (MSVC)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
else()
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../../wrappers/gdiplus ${CMAKE_BINARY_DIR}/ZXingGdiPlus)

if (ENABLE_DECODERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ../../common/TestReader.h
        ../../common/TestReader.cpp
        ../../common/Pdf417MultipleCodeReader.h
        ../../common/Pdf417MultipleCodeReader.cpp
        ../../common/BlackboxTestRunner.h
        ../../common/BlackboxTestRunner.cpp
    )

    target_include_directories (ReaderTest
        PRIVATE ../../common
    )

    #set_target_properties (ReaderTest PROPERTIES
    #   LINK_FLAGS "/link setargv.obj"
    #)

    target_link_libraries (ReaderTest ZXingGdiPlus)
endif()

if (ENABLE_ENCODERS)
    add_executable (WriterTest
        TestWriterMain.cpp
    )

	target_include_directories (WriterTest
		PRIVATE ../../common
	)
    
    #set_target_properties (WriterTest PROPERTIES
    #   LINK_FLAGS "/link setargv.obj"
    #)

    target_link_libraries (WriterTest ZXingGdiPlus)
endif()
