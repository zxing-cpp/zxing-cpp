cmake_minimum_required(VERSION 3.14)

# Download and unpack googletest at configure time
include(FetchContent)
#set(FETCHCONTENT_QUIET Off)
FetchContent_Declare (googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x)
# Prevent overriding the parent project's compiler/linker settings on Windows
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable (googletest)

# don't install gtest stuff on "make install"
set (INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# Our executable
add_executable (UnitTest
    BarcodeFormatTest.cpp
    BitArrayUtility.h
    BitArrayUtility.cpp
    PseudoRandom.h
    BitHacksTest.cpp
    ReedSolomonTest.cpp
    TextUtfEncodingTest.cpp
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
    oned/ODCodaBarWriterTest.cpp
    oned/ODCode39ExtendedModeTest.cpp
    oned/ODCode39WriterTest.cpp
    oned/ODCode93ReaderTest.cpp
    oned/ODCode93WriterTest.cpp
    oned/ODCode128WriterTest.cpp
    oned/ODEAN8WriterTest.cpp
    oned/ODEAN13WriterTest.cpp
    oned/ODITFWriterTest.cpp
    oned/ODUPCAWriterTest.cpp
    oned/ODUPCEWriterTest.cpp
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
    pdf417/PDF417WriterTest.cpp
)

target_include_directories (UnitTest PRIVATE .)

target_link_libraries (UnitTest ZXing::ZXing gtest_main)

add_test(NAME UnitTest COMMAND UnitTest)
