zxing_add_package_stb()

option (ZXING_EXAMPLES_USE_WEBP "Add support for WebP image decoding" OFF)

if (ZXING_EXAMPLES_USE_WEBP)
    zxing_add_package(WebP WebP https://github.com/webmproject/libwebp.git v1.6.0)
endif()

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
    if (ZXING_EXAMPLES_USE_WEBP)
        target_link_libraries (ZXingReader ZXing::ZXing stb::stb WebP::webp)
        target_compile_definitions(ZXingReader PRIVATE ZXING_USE_WEBP)
    endif()

    add_test(NAME ZXingReaderTest COMMAND ZXingReader -fast -format qrcode test.png) # see above
    set_tests_properties(ZXingReaderTest PROPERTIES DEPENDS ZXingWriterTest)

    if (ZXING_EXAMPLES_USE_WEBP AND WIN32)
        add_custom_command(TARGET ZXingReader POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:ZXingReader>
                $<TARGET_FILE_DIR:ZXingReader>
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()

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
