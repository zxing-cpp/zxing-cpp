set (CMAKE_CXX_STANDARD 17)

if (BUILD_READERS)
    add_executable (ReaderTest
        TestReaderMain.cpp
        ImageLoader.h
        ImageLoader.cpp
        Pdf417MultipleCodeReader.h
        Pdf417MultipleCodeReader.cpp
        QRCodeStructuredAppendReader.h
        QRCodeStructuredAppendReader.cpp
        BlackboxTestRunner.h
        BlackboxTestRunner.cpp
        ZXFilesystem.h
    )

    target_include_directories (ReaderTest PRIVATE ../../thirdparty/stb)

    target_link_libraries(ReaderTest
        ZXing::ZXing
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    )

    add_test(NAME ReaderTest COMMAND ReaderTest ${CMAKE_CURRENT_SOURCE_DIR}/../samples)
endif()

if (BUILD_WRITERS)
    add_executable (WriterTest TestWriterMain.cpp)

    target_include_directories (WriterTest PRIVATE ../../thirdparty/stb)

    target_link_libraries (WriterTest ZXing::ZXing)

    add_test(NAME WriterTest COMMAND WriterTest)
endif()
