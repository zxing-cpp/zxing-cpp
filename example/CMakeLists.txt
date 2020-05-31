cmake_minimum_required (VERSION 3.1.3)

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
