
if (ENABLE_DECODERS)
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

    target_compile_features(ReaderTest PRIVATE cxx_std_17)
    # the above line seems to have no effect on the current github-actions combination of cmake and VisualStudio
    if (MSVC_VERSION GREATER_EQUAL "1900")
        target_compile_options(ReaderTest PRIVATE "/std:c++17")
    endif()

    target_include_directories (ReaderTest PRIVATE ../../thirdparty/stb)

    target_link_libraries(ReaderTest
        ZXing::Core
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
    )
endif()

if (ENABLE_ENCODERS)
    add_executable (WriterTest TestWriterMain.cpp)

    target_include_directories (WriterTest PRIVATE ../../thirdparty/stb)

    target_link_libraries (WriterTest ZXingCore)
endif()
