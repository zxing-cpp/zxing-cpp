zxing_add_package_stb()

if (BUILD_READERS)
    add_executable (zxing-c-test zxing-c-test.c)
    target_link_libraries (zxing-c-test ZXing::ZXing stb::stb)
    add_test(NAME zxing-c-test COMMAND zxing-c-test ${CMAKE_SOURCE_DIR}/test/samples/qrcode-1/1.png)
endif()
