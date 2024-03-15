zxing_add_package_stb()

if (ZXING_READERS)
    add_executable (ZXingCTest ZXingCTest.c)
    target_link_libraries (ZXingCTest ZXing::ZXing stb::stb)
    add_test(NAME ZXingCTest COMMAND ZXingCTest ${CMAKE_SOURCE_DIR}/test/samples/qrcode-1/1.png)
endif()
