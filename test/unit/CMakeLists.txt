cmake_minimum_required (VERSION 3.1.3)

project (ZXingUnitTest)

# Enable both reader and writer for testing
set (ENABLE_ENCODERS ON CACHE INTERNAL "Include encoders" FORCE)
set (ENABLE_DECODERS ON CACHE INTERNAL "include decoders" FORCE)

add_definitions (-DUNICODE -D_UNICODE -DZXING_BUILD_FOR_TEST)

if (MSVC)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GS-")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GS-")
else()
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
endif()


# Download and unpack googletest at configure time
configure_file (CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process (COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if (result)
  message (FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process (COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if (result)
  message (FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set (gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory (${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build)

# Add ZXingCore dependency
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/ZXingCore)

SET (COMMON_TEST_UTILITY
    ../common/BitArrayUtility.h
    ../common/BitArrayUtility.cpp
    ../common/BitMatrixUtility.h
    ../common/BitMatrixUtility.cpp
    ../common/ByteMatrixUtility.h
    ../common/ByteMatrixUtility.cpp
    ../common/PseudoRandom.h
)

# Our executable
add_executable (ZXingUnitTest
    ${COMMON_TEST_UTILITY}
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
    BitHacksTest.cpp
)

target_include_directories (ZXingUnitTest
    PRIVATE ../common
)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    target_compile_options (ZXingUnitTest PRIVATE -std=c++11)
elseif (APPLE)
    target_compile_options (ZXingUnitTest PRIVATE -std=c++11 -stdlib=libc++)
endif()

target_link_libraries (ZXingUnitTest
    gtest_main
    ZXingCore
)
