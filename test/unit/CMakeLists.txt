zxing_add_package(GTest googletest https://github.com/google/googletest.git v1.15.2)

if (GTest_POPULATED)
    # don't install gtest stuff on "make install"
    set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
endif()

if (MSVC)
    # default to UTF-8
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

add_executable (UnitTest
    BarcodeFormatTest.cpp
    BitArrayUtility.cpp
    BitArrayUtility.h
    BitHacksTest.cpp
    CharacterSetECITest.cpp
    ErrorTest.cpp
    GTINTest.cpp
    PseudoRandom.h
    SanitizerSupport.cpp
    TextUtfEncodingTest.cpp
    ZXAlgorithmsTest.cpp
)

if (ZXING_READERS)
target_sources (UnitTest PRIVATE
    GS1Test.cpp
    PatternTest.cpp
    TextDecoderTest.cpp
    ThresholdBinarizerTest.cpp
    aztec/AZDecoderTest.cpp
    aztec/AZDetectorTest.cpp
    datamatrix/DMDecodedBitStreamParserTest.cpp
    maxicode/MCDecoderTest.cpp
    oned/ODCode128ReaderTest.cpp
    oned/ODCode39ExtendedModeTest.cpp
    oned/ODCode39ReaderTest.cpp
    oned/ODCode93ReaderTest.cpp
    oned/ODDataBarExpandedBitDecoderTest.cpp
    oned/ODDataBarReaderTest.cpp
    pdf417/PDF417DecoderTest.cpp
    pdf417/PDF417ErrorCorrectionTest.cpp
    pdf417/PDF417ScanningDecoderTest.cpp
    qrcode/MQRDecoderTest.cpp
    qrcode/QRBitMatrixParserTest.cpp
    qrcode/QRDataMaskTest.cpp
    qrcode/QRDecodedBitStreamParserTest.cpp
    qrcode/QRErrorCorrectionLevelTest.cpp
    qrcode/QRFormatInformationTest.cpp
    qrcode/QRModeTest.cpp
    qrcode/QRVersionTest.cpp
    qrcode/RMQRDecoderTest.cpp
)
endif()

if (ZXING_WRITERS MATCHES "ON|OLD|BOTH")
target_sources (UnitTest PRIVATE
    aztec/AZEncoderTest.cpp
    datamatrix/DMHighLevelEncodeTest.cpp
    datamatrix/DMPlacementTest.cpp
    datamatrix/DMSymbolInfoTest.cpp
    datamatrix/DMWriterTest.cpp
    oned/ODCode39WriterTest.cpp
    oned/ODCode93WriterTest.cpp
    oned/ODEAN13WriterTest.cpp
    oned/ODEAN8WriterTest.cpp
    oned/ODITFWriterTest.cpp
    oned/ODUPCAWriterTest.cpp
    oned/ODUPCEWriterTest.cpp
    pdf417/PDF417HighLevelEncoderTest.cpp
    pdf417/PDF417WriterTest.cpp
    qrcode/QRWriterTest.cpp
)
endif()

if (ZXING_READERS AND ZXING_WRITERS MATCHES "ON|OLD|BOTH")
target_sources (UnitTest PRIVATE
    ContentTest.cpp
    ReedSolomonTest.cpp
    TextEncoderTest.cpp
    aztec/AZEncodeDecodeTest.cpp
    aztec/AZHighLevelEncoderTest.cpp
    datamatrix/DMEncodeDecodeTest.cpp
    oned/ODCodaBarWriterTest.cpp
    oned/ODCode128WriterTest.cpp
    qrcode/QREncoderTest.cpp
)
endif()

target_include_directories (UnitTest PRIVATE .)

target_link_libraries (UnitTest ZXing::ZXing GTest::gtest_main GTest::gmock)

#target_precompile_headers(UnitTest PRIVATE ${CMAKE_SOURCE_DIR}/core/src/ReadBarcode.h)

add_test(NAME UnitTest COMMAND UnitTest)
