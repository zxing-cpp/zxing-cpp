zxing_add_package_stb()
zxing_add_package(WebP WebP https://github.com/webmproject/libwebp.git v1.6.0)

if (ZXING_READERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ImageLoader.h
        ImageLoader.cpp
        BlackboxTestRunner.h
        BlackboxTestRunner.cpp
    )

    target_link_libraries(ReaderTest
        ZXing::ZXing stb::stb WebP::webp
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    )

    if (WIN32)
        add_custom_command(TARGET ReaderTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:ReaderTest>
                $<TARGET_FILE_DIR:ReaderTest>
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()

    add_test(NAME ReaderTest COMMAND ReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/../samples)
endif()

if (ZXING_WRITERS)
    add_executable (WriterTest TestWriterMain.cpp)

    target_link_libraries (WriterTest ZXing::ZXing stb::stb)

    add_test(NAME WriterTest COMMAND WriterTest)
endif()
