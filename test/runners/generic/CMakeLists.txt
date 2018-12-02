cmake_minimum_required (VERSION 2.8.9)

project (ZXingTest)

set (ENABLE_ENCODERS OFF CACHE BOOL "Check to include encoders")
set (ENABLE_DECODERS ON CACHE BOOL "Check to include decoders")

add_definitions (-DUNICODE -D_UNICODE -DLODEPNG_NO_COMPILE_ENCODER)

if (MSVC)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
else()
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lstdc++fs")
    elseif (APPLE)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    endif()
endif()

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../../core ${CMAKE_BINARY_DIR}/ZXingCore)

if (ENABLE_DECODERS)
    find_package(Boost COMPONENTS system filesystem REQUIRED)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ../../common/TestReader.cpp
        ../../common/BlackboxTestRunner.cpp
        ../../common/lodepng.cpp
    )
    
    target_include_directories (ReaderTest
        PRIVATE ../../common
    )

    target_link_libraries (ReaderTest ZXingCore
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
    )
endif()

#if (ENABLE_ENCODERS)
#    add_executable (WriterTest
#        TestWriterMain.cpp
#    )
#
#    target_link_libraries (WriterTest ZXingCore)
#endif()
