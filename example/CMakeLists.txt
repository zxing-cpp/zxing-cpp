#show that it is possible to use the library (in its simples form) with a c++-11 compiler
set (CMAKE_CXX_STANDARD 11)

if (BUILD_READERS)
    add_executable (ZXingReader ZXingReader.cpp)

    target_include_directories (ZXingReader PRIVATE ../thirdparty/stb)

    target_link_libraries (ZXingReader ZXing::Core)
endif()

if (BUILD_WRITERS)
    add_executable (ZXingWriter ZXingWriter.cpp)

    target_include_directories (ZXingWriter PRIVATE ../thirdparty/stb)

    target_link_libraries (ZXingWriter ZXing::Core)
endif()
