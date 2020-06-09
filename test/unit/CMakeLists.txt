
# Download and unpack googletest at configure time
configure_file (CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process (COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if (result)
    message (FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process (COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if (result)
    message (FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory (${CMAKE_CURRENT_BINARY_DIR}/googletest-src ${CMAKE_CURRENT_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)


# Our executable
add_executable (UnitTest
    BarcodeFormatTest.cpp
    BitArrayUtility.h
    BitArrayUtility.cpp
    PseudoRandom.h
    BitHacksTest.cpp
    ReedSolomonTest.cpp
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
