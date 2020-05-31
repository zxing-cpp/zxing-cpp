#show that it is possible to use the library (in its simples form) with a c++-11 compiler
set (CMAKE_CXX_STANDARD 11)

if (ENABLE_DECODERS)
    add_executable (scan_image scan_image.cpp)

    target_include_directories (scan_image PRIVATE ../thirdparty/stb)

    target_link_libraries (scan_image ZXing::Core)
endif()

if (ENABLE_ENCODERS)
    add_executable (generate_image generate_image.cpp)

    target_include_directories (generate_image PRIVATE ../thirdparty/stb)

    target_link_libraries (generate_image ZXing::Core)
endif()
