zxing_add_package_stb()

include (GNUInstallDirs)

if (ZXING_WRITERS AND ZXING_ENABLE_QRCODE)
    add_executable (ZXingWriter ZXingWriter.cpp)

    target_link_libraries (ZXingWriter ZXing::ZXing stb::stb)

    add_test(NAME ZXingWriterTest COMMAND ZXingWriter qrcode "I have the best words." test.png)

    install(TARGETS ZXingWriter DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (ZXING_READERS)
    add_executable (ZXingReader ZXingReader.cpp)

    target_link_libraries (ZXingReader ZXing::ZXing stb::stb)

    add_test(NAME ZXingReaderTest COMMAND ZXingReader -fast -format qrcode test.png) # see above
    set_tests_properties(ZXingReaderTest PROPERTIES DEPENDS ZXingWriterTest)

    install(TARGETS ZXingReader DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (ZXING_READERS)
    find_package(OpenCV QUIET)
    if (OpenCV_FOUND)
        add_executable (ZXingOpenCV ZXingOpenCV.cpp)
        target_include_directories (ZXingOpenCV PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries (ZXingOpenCV ZXing::ZXing ${OpenCV_LIBS})
    else ()
        message("INFO: OpenCV not found, skipping ZXingOpenCV example")
    endif()
endif()

add_executable (ZXingBarcodeFormat ZXingBarcodeFormat.cpp)
target_link_libraries (ZXingBarcodeFormat ZXing::ZXing)
