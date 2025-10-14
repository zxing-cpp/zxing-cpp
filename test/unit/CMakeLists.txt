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
    ContentTest.cpp
    ErrorTest.cpp
    GTINTest.cpp
    JSONTest.cpp
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
    $<$<BOOL:${ZXING_ENABLE_1D}>:ThresholdBinarizerTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_AZTEC}>:aztec/AZDecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_AZTEC}>:aztec/AZDetectorTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMDecodedBitStreamParserTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_MAXICODE}>:maxicode/MCDecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode128ReaderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode39ExtendedModeTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode39ReaderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode93ReaderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODDataBarExpandedBitDecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODDataBarReaderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_PDF417}>:pdf417/PDF417DecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_PDF417}>:pdf417/PDF417ErrorCorrectionTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_PDF417}>:pdf417/PDF417ScanningDecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/MQRDecoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRBitMatrixParserTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRDataMaskTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRDecodedBitStreamParserTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRErrorCorrectionLevelTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRFormatInformationTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRModeTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRVersionTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/RMQRDecoderTest.cpp>
)
endif()

if (ZXING_WRITERS MATCHES "ON|OLD|BOTH")
target_sources (UnitTest PRIVATE
    $<$<BOOL:${ZXING_ENABLE_AZTEC}>:aztec/AZEncoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMHighLevelEncodeTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMPlacementTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMSymbolInfoTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMWriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode39WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode93WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODEAN13WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODEAN8WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODITFWriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODUPCAWriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODUPCEWriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_PDF417}>:pdf417/PDF417HighLevelEncoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_PDF417}>:pdf417/PDF417WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QRWriterTest.cpp>
)
endif()

if (ZXING_READERS AND ZXING_WRITERS MATCHES "ON|OLD|BOTH")
target_sources (UnitTest PRIVATE
    ReedSolomonTest.cpp
    TextEncoderTest.cpp
    $<$<BOOL:${ZXING_ENABLE_AZTEC}>:aztec/AZEncodeDecodeTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_AZTEC}>:aztec/AZHighLevelEncoderTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_DATAMATRIX}>:datamatrix/DMEncodeDecodeTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCodaBarWriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_1D}>:oned/ODCode128WriterTest.cpp>
    $<$<BOOL:${ZXING_ENABLE_QRCODE}>:qrcode/QREncoderTest.cpp>
)
endif()

if (ZXING_WRITERS MATCHES "NEW|BOTH")
target_sources (UnitTest PRIVATE
    WriteBarcodeTest.cpp
)
endif()

target_include_directories (UnitTest PRIVATE .)

target_link_libraries (UnitTest ZXing::ZXing GTest::gtest_main GTest::gmock)

#target_precompile_headers(UnitTest PRIVATE ${CMAKE_SOURCE_DIR}/core/src/ReadBarcode.h)

add_test(NAME UnitTest COMMAND UnitTest)
