zxing_add_package(GTest googletest https://github.com/google/googletest.git release-1.11.0)

if (GTest_POPULATED)
    # don't install gtest stuff on "make install"
    set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)
endif()

# Our executable
add_executable (UnitTest
    BarcodeFormatTest.cpp
    BitArrayUtility.h
    BitArrayUtility.cpp
    PseudoRandom.h
    BitHacksTest.cpp
    CharacterSetECITest.cpp
    ContentTest.cpp
    ErrorTest.cpp
    GTINTest.cpp
    GS1Test.cpp
    PatternTest.cpp
    ReedSolomonTest.cpp
    SanitizerSupport.cpp
    TextDecoderTest.cpp
    TextEncoderTest.cpp
    TextUtfEncodingTest.cpp
    ThresholdBinarizerTest.cpp
    ZXAlgorithmsTest.cpp
    aztec/AZDetectorTest.cpp
    aztec/AZDecoderTest.cpp
    aztec/AZEncoderTest.cpp
    aztec/AZEncodeDecodeTest.cpp
    aztec/AZHighLevelEncoderTest.cpp
    datamatrix/DMDecodedBitStreamParserTest.cpp
    datamatrix/DMEncodeDecodeTest.cpp
    datamatrix/DMHighLevelEncodeTest.cpp
    datamatrix/DMPlacementTest.cpp
    datamatrix/DMSymbolInfoTest.cpp
    datamatrix/DMWriterTest.cpp
    maxicode/MCDecoderTest.cpp
    oned/ODCodaBarWriterTest.cpp
    oned/ODCode39ExtendedModeTest.cpp
    oned/ODCode39ReaderTest.cpp
    oned/ODCode39WriterTest.cpp
    oned/ODCode93ReaderTest.cpp
    oned/ODCode93WriterTest.cpp
    oned/ODCode128ReaderTest.cpp
    oned/ODCode128WriterTest.cpp
    oned/ODDataBarReaderTest.cpp
    oned/ODDataBarExpandedBitDecoderTest.cpp
    oned/ODEAN8WriterTest.cpp
    oned/ODEAN13WriterTest.cpp
    oned/ODITFWriterTest.cpp
    oned/ODUPCAWriterTest.cpp
    oned/ODUPCEWriterTest.cpp
    qrcode/MQRDecoderTest.cpp
    qrcode/QRBitMatrixParserTest.cpp
    qrcode/QRDataMaskTest.cpp
    qrcode/QRDecodedBitStreamParserTest.cpp
    qrcode/QREncoderTest.cpp
    qrcode/QRErrorCorrectionLevelTest.cpp
    qrcode/QRFormatInformationTest.cpp
    qrcode/QRModeTest.cpp
    qrcode/QRVersionTest.cpp
    qrcode/QRWriterTest.cpp
    pdf417/PDF417DecoderTest.cpp
    pdf417/PDF417ErrorCorrectionTest.cpp
    pdf417/PDF417HighLevelEncoderTest.cpp
    pdf417/PDF417ScanningDecoderTest.cpp
    pdf417/PDF417WriterTest.cpp
)

target_include_directories (UnitTest PRIVATE .)

target_link_libraries (UnitTest ZXing::ZXing GTest::gtest_main GTest::gmock)

add_test(NAME UnitTest COMMAND UnitTest)
