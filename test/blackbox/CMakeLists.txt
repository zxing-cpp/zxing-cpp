zxing_add_package_stb()
zxing_add_package(fmt fmtlib https://github.com/fmtlib/fmt.git 11.1.1)

if (ZXING_READERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ImageLoader.h
        ImageLoader.cpp
        BlackboxTestRunner.h
        BlackboxTestRunner.cpp
        ZXFilesystem.h
    )

    target_link_libraries(ReaderTest
        ZXing::ZXing fmt::fmt stb::stb
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    )

    add_test(NAME ReaderTest COMMAND ReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/../samples)
endif()

if (ZXING_WRITERS)
    add_executable (WriterTest TestWriterMain.cpp)

    target_link_libraries (WriterTest ZXing::ZXing stb::stb)

    add_test(NAME WriterTest COMMAND WriterTest)
endif()
