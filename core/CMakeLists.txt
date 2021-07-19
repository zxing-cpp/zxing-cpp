cmake_minimum_required (VERSION 3.10)

if (NOT DEFINED BUILD_WRITERS)
    set (BUILD_WRITERS OFF)
endif()

if (NOT DEFINED BUILD_READERS)
    set (BUILD_READERS ON)
endif()

set (ZXING_CORE_DEFINES)
if (WINRT)
    set (ZXING_CORE_DEFINES ${ZXING_CORE_DEFINES}
        -DWINRT
    )
endif()

set (ZXING_CORE_LOCAL_DEFINES)
if (MSVC)
    set (ZXING_CORE_LOCAL_DEFINES ${ZXING_CORE_LOCAL_DEFINES}
        -D_SCL_SECURE_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS
        -DNOMINMAX
    )
else()
    set (ZXING_CORE_LOCAL_DEFINES ${ZXING_CORE_LOCAL_DEFINES}
        -Wall -Wextra -Wno-missing-braces -Werror=undef)
endif()


################# Source files

set (COMMON_FILES
    src/BarcodeFormat.h
    src/BarcodeFormat.cpp
    src/BitArray.h
    src/BitArray.cpp
    src/BitHacks.h
    src/BitMatrix.h
    src/BitMatrix.cpp
    src/BitMatrixCursor.h
    src/BitMatrixIO.h
    src/BitMatrixIO.cpp
    src/ByteArray.h
    src/ByteMatrix.h
    src/CharacterSet.h
    src/CharacterSetECI.h
    src/CharacterSetECI.cpp
    src/ConcentricFinder.h
    src/ConcentricFinder.cpp
    src/CustomData.h
    src/Flags.h
    src/GenericGF.h
    src/GenericGF.cpp
    src/GenericGFPoly.h
    src/GenericGFPoly.cpp
    src/GTIN.h
    src/GTIN.cpp
    src/LogMatrix.h
    src/Matrix.h
    src/Pattern.h
    src/Point.h
    src/Quadrilateral.h
    src/RegressionLine.h
    src/Scope.h
    src/TextUtfEncoding.h
    src/TextUtfEncoding.cpp
    src/TritMatrix.h
    src/ZXBigInteger.h
    src/ZXBigInteger.cpp
    src/ZXConfig.h
    src/ZXNullable.h
    src/ZXContainerAlgorithms.h
    src/ZXTestSupport.h
)
if (BUILD_READERS)
    set (COMMON_FILES ${COMMON_FILES}
        src/BinaryBitmap.h
        src/BinaryBitmap.cpp
        src/BitSource.h
        src/BitSource.cpp
        src/DecodeHints.h
        src/DecodeHints.cpp
        src/DecodeStatus.h
        src/DecodeStatus.cpp
        src/DecoderResult.h
        src/DetectorResult.h
        src/GlobalHistogramBinarizer.h
        src/GlobalHistogramBinarizer.cpp
        src/GridSampler.h
        src/GridSampler.cpp
        src/HybridBinarizer.h
        src/HybridBinarizer.cpp
        src/ImageView.h
        src/MultiFormatReader.h
        src/MultiFormatReader.cpp
        src/PerspectiveTransform.h
        src/PerspectiveTransform.cpp
        src/Reader.h
        src/ReadBarcode.h
        src/ReadBarcode.cpp
        src/ReedSolomonDecoder.h
        src/ReedSolomonDecoder.cpp
        src/Result.h
        src/Result.cpp
        src/ResultPoint.h
        src/ResultPoint.cpp
        src/TextDecoder.h
        src/TextDecoder.cpp
        src/ThresholdBinarizer.h
        src/WhiteRectDetector.h
        src/WhiteRectDetector.cpp
    )
endif()
if (BUILD_WRITERS)
    set (COMMON_FILES ${COMMON_FILES}
        src/ByteMatrix.h
        src/ReedSolomonEncoder.h
        src/ReedSolomonEncoder.cpp
        src/TextEncoder.h
        src/TextEncoder.cpp
        src/MultiFormatWriter.h
        src/MultiFormatWriter.cpp
    )
endif()


set (AZTEC_FILES
)
if (BUILD_READERS)
    set (AZTEC_FILES ${AZTEC_FILES}
        src/aztec/AZDecoder.h
        src/aztec/AZDecoder.cpp
        src/aztec/AZDetector.h
        src/aztec/AZDetector.cpp
        src/aztec/AZDetectorResult.h
        src/aztec/AZReader.h
        src/aztec/AZReader.cpp
    )
endif()
if (BUILD_WRITERS)
    set (AZTEC_FILES ${AZTEC_FILES}
        src/aztec/AZEncodingState.h
        src/aztec/AZEncoder.h
        src/aztec/AZEncoder.cpp
        src/aztec/AZHighLevelEncoder.h
        src/aztec/AZHighLevelEncoder.cpp
        src/aztec/AZToken.h
        src/aztec/AZToken.cpp
        src/aztec/AZWriter.h
        src/aztec/AZWriter.cpp
    )
endif()


set (DATAMATRIX_FILES
    src/datamatrix/DMBitLayout.h
    src/datamatrix/DMBitLayout.cpp
)
if (BUILD_READERS)
    set (DATAMATRIX_FILES ${DATAMATRIX_FILES}
        src/datamatrix/DMDataBlock.h
        src/datamatrix/DMDataBlock.cpp
        src/datamatrix/DMDecoder.h
        src/datamatrix/DMDecoder.cpp
        src/datamatrix/DMDetector.h
        src/datamatrix/DMDetector.cpp
        src/datamatrix/DMVersion.h
        src/datamatrix/DMVersion.cpp
        src/datamatrix/DMReader.h
        src/datamatrix/DMReader.cpp
    )
endif()
if (BUILD_WRITERS)
    set (DATAMATRIX_FILES ${DATAMATRIX_FILES}
        src/datamatrix/DMECEncoder.h
        src/datamatrix/DMECEncoder.cpp
        src/datamatrix/DMEncoderContext.h
        src/datamatrix/DMHighLevelEncoder.h
        src/datamatrix/DMHighLevelEncoder.cpp
        src/datamatrix/DMSymbolInfo.h
        src/datamatrix/DMSymbolInfo.cpp
        src/datamatrix/DMSymbolShape.h
        src/datamatrix/DMWriter.h
        src/datamatrix/DMWriter.cpp
    )
endif()


set (MAXICODE_FILES
)
if (BUILD_READERS)
    set (MAXICODE_FILES ${MAXICODE_FILES}
        src/maxicode/MCBitMatrixParser.h
        src/maxicode/MCBitMatrixParser.cpp
        src/maxicode/MCDecoder.h
        src/maxicode/MCDecoder.cpp
        src/maxicode/MCReader.h
        src/maxicode/MCReader.cpp
    )
endif()


set (ONED_FILES
    src/oned/ODUPCEANCommon.h
    src/oned/ODUPCEANCommon.cpp
    src/oned/ODCode128Patterns.h
    src/oned/ODCode128Patterns.cpp
)
if (BUILD_READERS)
    set (ONED_FILES ${ONED_FILES}
        src/oned/ODCodabarReader.h
        src/oned/ODCodabarReader.cpp
        src/oned/ODCode39Reader.h
        src/oned/ODCode39Reader.cpp
        src/oned/ODCode93Reader.h
        src/oned/ODCode93Reader.cpp
        src/oned/ODCode128Reader.h
        src/oned/ODCode128Reader.cpp
        src/oned/ODDataBarCommon.h
        src/oned/ODDataBarCommon.cpp
        src/oned/ODDataBarReader.h
        src/oned/ODDataBarReader.cpp
        src/oned/ODDataBarExpandedReader.h
        src/oned/ODDataBarExpandedReader.cpp
        src/oned/ODITFReader.h
        src/oned/ODITFReader.cpp
        src/oned/ODMultiUPCEANReader.h
        src/oned/ODMultiUPCEANReader.cpp
        src/oned/ODReader.h
        src/oned/ODReader.cpp
        src/oned/ODRowReader.h
        src/oned/ODRowReader.cpp
    )
endif()
if (BUILD_WRITERS)
    set (ONED_FILES ${ONED_FILES}
        src/oned/ODCodabarWriter.h
        src/oned/ODCodabarWriter.cpp
        src/oned/ODCode39Writer.h
        src/oned/ODCode39Writer.cpp
        src/oned/ODCode93Writer.h
        src/oned/ODCode93Writer.cpp
        src/oned/ODCode128Writer.h
        src/oned/ODCode128Writer.cpp
        src/oned/ODEAN8Writer.h
        src/oned/ODEAN8Writer.cpp
        src/oned/ODEAN13Writer.h
        src/oned/ODEAN13Writer.cpp
        src/oned/ODITFWriter.h
        src/oned/ODITFWriter.cpp
        src/oned/ODUPCEWriter.h
        src/oned/ODUPCEWriter.cpp
        src/oned/ODUPCAWriter.h
        src/oned/ODUPCAWriter.cpp
        src/oned/ODWriterHelper.h
        src/oned/ODWriterHelper.cpp
    )
endif()


set (ONED_RSS_FILES
)
if (BUILD_READERS)
    set (ONED_RSS_FILES ${ONED_RSS_FILES}
        src/oned/rss/ODRSSExpandedBinaryDecoder.h
        src/oned/rss/ODRSSExpandedBinaryDecoder.cpp
        src/oned/rss/ODRSSFieldParser.h
        src/oned/rss/ODRSSFieldParser.cpp
        src/oned/rss/ODRSSGenericAppIdDecoder.h
        src/oned/rss/ODRSSGenericAppIdDecoder.cpp
    )
endif()


set (PDF417_FILES
)
if (BUILD_READERS)
    set (PDF417_FILES ${PDF417_FILES}
        src/pdf417/PDFBarcodeMetadata.h
        src/pdf417/PDFBarcodeValue.h
        src/pdf417/PDFBarcodeValue.cpp
        src/pdf417/PDFBoundingBox.h
        src/pdf417/PDFBoundingBox.cpp
        src/pdf417/PDFCodeword.h
        src/pdf417/PDFCodewordDecoder.h
        src/pdf417/PDFCodewordDecoder.cpp
        src/pdf417/PDFDecodedBitStreamParser.h
        src/pdf417/PDFDecodedBitStreamParser.cpp
        src/pdf417/PDFDecoderResultExtra.h
        src/pdf417/PDFDetectionResult.h
        src/pdf417/PDFDetectionResult.cpp
        src/pdf417/PDFDetectionResultColumn.h
        src/pdf417/PDFDetectionResultColumn.cpp
        src/pdf417/PDFDetector.h
        src/pdf417/PDFDetector.cpp
        src/pdf417/PDFModulusGF.h
        src/pdf417/PDFModulusGF.cpp
        src/pdf417/PDFModulusPoly.h
        src/pdf417/PDFModulusPoly.cpp
        src/pdf417/PDFReader.h
        src/pdf417/PDFReader.cpp
        src/pdf417/PDFScanningDecoder.h
        src/pdf417/PDFScanningDecoder.cpp
    )
endif()
if (BUILD_WRITERS)
    set (PDF417_FILES ${PDF417_FILES}
        src/pdf417/PDFCompaction.h
        src/pdf417/PDFEncoder.h
        src/pdf417/PDFEncoder.cpp
        src/pdf417/PDFHighLevelEncoder.h
        src/pdf417/PDFHighLevelEncoder.cpp
        src/pdf417/PDFWriter.h
        src/pdf417/PDFWriter.cpp
    )
endif()


set (QRCODE_FILES
    src/qrcode/QRCodecMode.h
    src/qrcode/QRCodecMode.cpp
    src/qrcode/QRErrorCorrectionLevel.h
    src/qrcode/QRErrorCorrectionLevel.cpp
    src/qrcode/QRVersion.h
    src/qrcode/QRVersion.cpp
)
if (BUILD_READERS)
    set (QRCODE_FILES ${QRCODE_FILES}
        src/qrcode/QRBitMatrixParser.h
        src/qrcode/QRBitMatrixParser.cpp
        src/qrcode/QRDataBlock.h
        src/qrcode/QRDataBlock.cpp
        src/qrcode/QRDataMask.h
        src/qrcode/QRDecoder.h
        src/qrcode/QRDecoder.cpp
        src/qrcode/QRDecoderMetadata.h
        src/qrcode/QRDetector.h
        src/qrcode/QRDetector.cpp
        src/qrcode/QRECB.h
        src/qrcode/QRFormatInformation.h
        src/qrcode/QRFormatInformation.cpp
        src/qrcode/QRReader.h
        src/qrcode/QRReader.cpp
    )
endif()
if (BUILD_WRITERS)
    set (QRCODE_FILES ${QRCODE_FILES}
        src/qrcode/QREncoder.h
        src/qrcode/QREncoder.cpp
        src/qrcode/QREncodeResult.h
        src/qrcode/QRMaskUtil.h
        src/qrcode/QRMaskUtil.cpp
        src/qrcode/QRMatrixUtil.h
        src/qrcode/QRMatrixUtil.cpp
        src/qrcode/QRWriter.h
        src/qrcode/QRWriter.cpp
    )
endif()


set (TEXT_CODEC_FILES
    src/textcodec/Big5MapTable.h
    src/textcodec/Big5MapTable.cpp
    src/textcodec/KRHangulMapping.h
    src/textcodec/KRHangulMapping.cpp
)
if (BUILD_READERS)
    set (TEXT_CODEC_FILES ${TEXT_CODEC_FILES}
        src/textcodec/Big5TextDecoder.h
        src/textcodec/Big5TextDecoder.cpp
        src/textcodec/GBTextDecoder.h
        src/textcodec/GBTextDecoder.cpp
        src/textcodec/JPTextDecoder.h
        src/textcodec/JPTextDecoder.cpp
        src/textcodec/KRTextDecoder.h
        src/textcodec/KRTextDecoder.cpp
    )
endif()
if (BUILD_WRITERS)
    set (TEXT_CODEC_FILES ${TEXT_CODEC_FILES}
        src/textcodec/Big5TextEncoder.h
        src/textcodec/Big5TextEncoder.cpp
        src/textcodec/GBTextEncoder.h
        src/textcodec/GBTextEncoder.cpp
        src/textcodec/JPTextEncoder.h
        src/textcodec/JPTextEncoder.cpp
        src/textcodec/KRTextEncoder.h
        src/textcodec/KRTextEncoder.cpp
    )
endif()

source_group (Sources FILES ${COMMON_FILES})
source_group (Sources\\aztec FILES ${AZTEC_FILES})
source_group (Sources\\datamatrix FILES ${DATAMATRIX_FILES})
source_group (Sources\\maxicode FILES ${MAXICODE_FILES})
source_group (Sources\\oned FILES ${ONED_FILES})
source_group (Sources\\oned\\rss FILES ${ONED_RSS_FILES})
source_group (Sources\\pdf417 FILES ${PDF417_FILES})
source_group (Sources\\qrcode FILES ${QRCODE_FILES})
source_group (Sources\\textcodec FILES ${TEXT_CODEC_FILES})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library (ZXing
    ${COMMON_FILES}
    ${AZTEC_FILES}
    ${DATAMATRIX_FILES}
    ${MAXICODE_FILES}
    ${ONED_FILES}
    ${ONED_RSS_FILES}
    ${PDF417_FILES}
    ${QRCODE_FILES}
    ${TEXT_CODEC_FILES}
)

target_include_directories (ZXing
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    INTERFACE "$<INSTALL_INTERFACE:include>"
)

target_compile_options (ZXing
    PUBLIC ${ZXING_CORE_DEFINES}
    PRIVATE ${ZXING_CORE_LOCAL_DEFINES}
)

include (CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG ("-ffloat-store" COMPILER_NEEDS_FLOAT_STORE)
if (COMPILER_NEEDS_FLOAT_STORE)
    target_compile_options(ZXing PRIVATE
        -ffloat-store   # same floating point precision in all optimization levels
    )
endif()

# the lib needs a c++-17 compiler but can be used with a c++-11 compiler (see examples)
target_compile_features(ZXing PRIVATE cxx_std_17 INTERFACE cxx_std_11)

target_link_libraries (ZXing PRIVATE Threads::Threads)

add_library(ZXing::ZXing ALIAS ZXing)
# add the old alias as well, to keep old clients compiling
# note: this only affects client code that includes ZXing via sub_directory.
#       for clients using the exported target, see ZXingConfig.cmake.in
add_library(ZXing::Core ALIAS ZXing)

set_target_properties(ZXing PROPERTIES EXPORT_NAME ZXing)
# force position indepent code to be able to link it as static lib into a DLL (e.g. the python module)
set_target_properties(ZXing PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (PROJECT_VERSION)
    set_target_properties(ZXing PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(ZXing PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

include (GNUInstallDirs)

install (
    TARGETS ZXing EXPORT ZXingTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION include
)

if(MSVC)
    set_target_properties(ZXing PROPERTIES
        COMPILE_PDB_NAME ZXing
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ZXing.pdb
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            CONFIGURATIONS Debug RelWithDebInfo)
endif()
