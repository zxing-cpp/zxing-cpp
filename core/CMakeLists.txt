cmake_minimum_required (VERSION 3.1.3)

if (NOT DEFINED ENABLE_ENCODERS)
    set (ENABLE_ENCODERS OFF)
endif()

if (NOT DEFINED ENABLE_DECODERS)
    set (ENABLE_DECODERS ON)
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
        -Wall -Wextra -Wno-missing-braces)
endif()

set (CMAKE_CXX_STANDARD 14)


################# Source files

set (COMMON_FILES
    src/BarcodeFormat.h
    src/BarcodeFormat.cpp
    src/BitArray.h
    src/BitArray.cpp
    src/BitHacks.h
    src/BitMatrix.h
    src/BitMatrix.cpp
    src/ByteArray.h
    src/CharacterSet.h
    src/CharacterSetECI.h
    src/CharacterSetECI.cpp
    src/CustomData.h
    src/GenericGF.h
    src/GenericGF.cpp
    src/GenericGFPoly.h
    src/GenericGFPoly.cpp
    src/TextUtfEncoding.h
    src/TextUtfEncoding.cpp
    src/ZXBigInteger.h
    src/ZXBigInteger.cpp
    src/ZXConfig.h
    src/ZXFilesystem.h
    src/ZXNullable.h
    src/ZXNumeric.h
    src/ZXContainerAlgorithms.h
    src/ZXStrConvWorkaround.h
    src/ZXTestSupport.h
)
if (ENABLE_DECODERS)
    set (COMMON_FILES ${COMMON_FILES}
        src/BinaryBitmap.h
        src/BitSource.h
        src/BitSource.cpp
        src/DecodeHints.h
        src/DecodeHints.cpp
        src/DecodeStatus.h
        src/DecodeStatus.cpp
        src/DecoderResult.h
        src/DetectorResult.h
        src/GenericLuminanceSource.h
        src/GenericLuminanceSource.cpp
        src/GlobalHistogramBinarizer.h
        src/GlobalHistogramBinarizer.cpp
        src/GridSampler.h
        src/GridSampler.cpp
        src/HybridBinarizer.h
        src/HybridBinarizer.cpp
        src/LuminanceSource.h
        src/LuminanceSource.cpp
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
        src/ResultMetadata.h
        src/ResultMetadata.cpp
        src/ResultPoint.h
        src/ResultPoint.cpp
        src/TextDecoder.h
        src/TextDecoder.cpp
        src/WhiteRectDetector.h
        src/WhiteRectDetector.cpp
    )
endif()
if (ENABLE_ENCODERS)
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
if (ENABLE_DECODERS)
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
if (ENABLE_ENCODERS)
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
)
if (ENABLE_DECODERS)
    set (DATAMATRIX_FILES ${DATAMATRIX_FILES}
        src/datamatrix/DMBitMatrixParser.h
        src/datamatrix/DMBitMatrixParser.cpp
        src/datamatrix/DMECB.h
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
if (ENABLE_ENCODERS)
    set (DATAMATRIX_FILES ${DATAMATRIX_FILES}
        src/datamatrix/DMECEncoder.h
        src/datamatrix/DMECEncoder.cpp
        src/datamatrix/DMEncoderContext.h
        src/datamatrix/DMDefaultPlacement.h
        src/datamatrix/DMDefaultPlacement.cpp
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
if (ENABLE_DECODERS)
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
if (ENABLE_DECODERS)
    set (ONED_FILES ${ONED_FILES}
        src/oned/ODCodabarReader.h
        src/oned/ODCodabarReader.cpp
        src/oned/ODCode39Reader.h
        src/oned/ODCode39Reader.cpp
        src/oned/ODCode93Reader.h
        src/oned/ODCode93Reader.cpp
        src/oned/ODCode128Reader.h
        src/oned/ODCode128Reader.cpp
        src/oned/ODEAN8Reader.h
        src/oned/ODEAN8Reader.cpp
        src/oned/ODEAN13Reader.h
        src/oned/ODEAN13Reader.cpp
        src/oned/ODEANManufacturerOrgSupport.h
        src/oned/ODEANManufacturerOrgSupport.cpp
        src/oned/ODITFReader.h
        src/oned/ODITFReader.cpp
        src/oned/ODMultiUPCEANReader.h
        src/oned/ODMultiUPCEANReader.cpp
        src/oned/ODReader.h
        src/oned/ODReader.cpp
        src/oned/ODRowReader.h
        src/oned/ODRowReader.cpp
        src/oned/ODRSS14Reader.h
        src/oned/ODRSS14Reader.cpp
        src/oned/ODRSSExpandedReader.h
        src/oned/ODRSSExpandedReader.cpp
        src/oned/ODUPCAReader.h
        src/oned/ODUPCAReader.cpp
        src/oned/ODUPCEANExtensionSupport.h
        src/oned/ODUPCEANExtensionSupport.cpp
        src/oned/ODUPCEANReader.h
        src/oned/ODUPCEANReader.cpp
        src/oned/ODUPCEReader.h
        src/oned/ODUPCEReader.cpp
    )
endif()
if (ENABLE_ENCODERS)
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
if (ENABLE_DECODERS)
    set (ONED_RSS_FILES ${ONED_RSS_FILES}
        src/oned/rss/ODRSSDataCharacter.h
        src/oned/rss/ODRSSExpandedBinaryDecoder.h
        src/oned/rss/ODRSSExpandedBinaryDecoder.cpp
        src/oned/rss/ODRSSExpandedPair.h
        src/oned/rss/ODRSSExpandedRow.h
        src/oned/rss/ODRSSFieldParser.h
        src/oned/rss/ODRSSFieldParser.cpp
        src/oned/rss/ODRSSFinderPattern.h
        src/oned/rss/ODRSSGenericAppIdDecoder.h
        src/oned/rss/ODRSSGenericAppIdDecoder.cpp
        src/oned/rss/ODRSSPair.h
        src/oned/rss/ODRSSReaderHelper.h
        src/oned/rss/ODRSSReaderHelper.cpp
    )
endif()


set (PDF417_FILES
)
if (ENABLE_DECODERS)
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
if (ENABLE_ENCODERS)
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
if (ENABLE_DECODERS)
    set (QRCODE_FILES ${QRCODE_FILES}
        src/qrcode/QRAlignmentPattern.h
        src/qrcode/QRAlignmentPattern.cpp
        src/qrcode/QRAlignmentPatternFinder.h
        src/qrcode/QRAlignmentPatternFinder.cpp
        src/qrcode/QRBitMatrixParser.h
        src/qrcode/QRBitMatrixParser.cpp
        src/qrcode/QRDataBlock.h
        src/qrcode/QRDataBlock.cpp
        src/qrcode/QRDataMask.h
        src/qrcode/QRDataMask.cpp
        src/qrcode/QRDecoder.h
        src/qrcode/QRDecoder.cpp
        src/qrcode/QRDecoderMetadata.h
        src/qrcode/QRDetector.h
        src/qrcode/QRDetector.cpp
        src/qrcode/QRECB.h
        src/qrcode/QRFinderPattern.h
        src/qrcode/QRFinderPattern.cpp
        src/qrcode/QRFinderPatternFinder.h
        src/qrcode/QRFinderPatternFinder.cpp
        src/qrcode/QRFinderPatternInfo.h
        src/qrcode/QRFormatInformation.h
        src/qrcode/QRFormatInformation.cpp
        src/qrcode/QRReader.h
        src/qrcode/QRReader.cpp
    )
endif()
if (ENABLE_ENCODERS)
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
if (ENABLE_DECODERS)
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
if (ENABLE_ENCODERS)
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

find_package (Threads)

add_library (ZXingCore
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

target_include_directories (ZXingCore
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
    INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>"
)

target_compile_options (ZXingCore
    PUBLIC ${ZXING_CORE_DEFINES}
    PRIVATE ${ZXING_CORE_LOCAL_DEFINES}
)

include (CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG ("-ffloat-store" COMPILER_NEEDS_FLOAT_STORE)
if (COMPILER_NEEDS_FLOAT_STORE)
    target_compile_options(ZXingCore PRIVATE
        -ffloat-store   # same floating point precision in all optimization levels
    )
endif()

target_link_libraries (ZXingCore PUBLIC ${CMAKE_THREAD_LIBS_INIT})

add_library(ZXing::Core ALIAS ZXingCore)
set_target_properties(ZXingCore PROPERTIES EXPORT_NAME Core)

include (GNUInstallDirs)

# Once we can require cmake 1.13, then we can move this to ../CMakeLists.txt, see: https://gitlab.kitware.com/cmake/cmake/merge_requests/2152
install (
    TARGETS ZXingCore EXPORT ZXingTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION include
)
