zxing_add_package_stb()

if (BUILD_READERS)
    add_executable (zxing-c-test zxing-c.cpp zxing-c-test.c)
    target_link_libraries (zxing-c-test ZXing::ZXing stb::stb)
endif()
