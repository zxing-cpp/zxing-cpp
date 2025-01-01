cmake_minimum_required(VERSION 3.16)

project (ZXing VERSION "2.3.0")
set (ZXING_SONAME 3) # see https://github.com/zxing-cpp/zxing-cpp/issues/333

if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (DEFINED BUILD_READERS)
    set (ZXING_READERS ${BUILD_READERS})
    message (WARNING "zxing-cpp cmake options BUILD_... are deprecated, please switch to ZXING_... variant")
endif()
if (DEFINED BUILD_WRITERS)
    set (ZXING_WRITERS ${BUILD_WRITERS})
    message (WARNING "zxing-cpp cmake options BUILD_... are deprecated, please switch to ZXING_... variant")
endif()

if (NOT DEFINED ZXING_WRITERS)
    set (ZXING_WRITERS OFF)
endif()

if (NOT DEFINED ZXING_READERS)
    set (ZXING_READERS ON)
endif()

set (ZXING_WRITERS_NEW OFF)
set (ZXING_WRITERS_OLD OFF)
if (ZXING_WRITERS MATCHES "OLD|ON")
    set (ZXING_WRITERS ON)
    set (ZXING_WRITERS_OLD ON)
elseif (ZXING_WRITERS MATCHES "NEW")
    set (ZXING_WRITERS ON)
    set (ZXING_WRITERS_NEW ON)
elseif (ZXING_WRITERS MATCHES "BOTH")
    set (ZXING_WRITERS ON)
    set (ZXING_WRITERS_NEW ON)
    set (ZXING_WRITERS_OLD ON)
endif()

if (BUILD_SHARED_LIBS)
    set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set (ZXING_PUBLIC_FLAGS
    $<$<BOOL:${ZXING_EXPERIMENTAL_API}>:-DZXING_EXPERIMENTAL_API>
)
if (WINRT)
    set (ZXING_PUBLIC_FLAGS ${ZXING_PUBLIC_FLAGS}
        -DWINRT
    )
endif()
if (MSVC)
    set (ZXING_PUBLIC_FLAGS ${ZXING_PUBLIC_FLAGS}
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    )
endif()

set (ZXING_PRIVATE_FLAGS
    $<$<BOOL:${ZXING_WRITERS_NEW}>:-DZXING_USE_ZINT>
    $<$<BOOL:${ZXING_UNIT_TESTS}>:-DZXING_BUILD_FOR_TEST>
)
if (MSVC)
    set (ZXING_PRIVATE_FLAGS ${ZXING_PRIVATE_FLAGS}
        -D_SCL_SECURE_NO_WARNINGS
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_WARNINGS
        -DNOMINMAX
        /utf-8 # see https://github.com/zxing-cpp/zxing-cpp/issues/757
    )
else()
    set (ZXING_PRIVATE_FLAGS ${ZXING_PRIVATE_FLAGS}
        -Wall -Wextra -Wno-missing-braces -Werror=undef -Werror=return-type)
endif()

include (CheckCXXCompilerFlag)

# This is needed for reproducible builds across different build directories.
# Without this, the usage of the __FILE__ macro leaves the build directory in
# the binary. When building the Python extension with build isolation enabled
# this would lead to random paths in the binary.
set(FILE_PREFIX_ARG "-fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=")
check_cxx_compiler_flag("${FILE_PREFIX_ARG}" HAS_FILE_PREFIX_ARG)
if(HAS_FILE_PREFIX_ARG)
    set(ZXING_PRIVATE_FLAGS ${ZXING_PRIVATE_FLAGS} "${FILE_PREFIX_ARG}")
endif()

################# Source files

set (COMMON_FILES
    src/Barcode.h
    src/Barcode.cpp
    src/BarcodeFormat.h
    src/BarcodeFormat.cpp
    src/BitHacks.h
    src/BitMatrix.h
    src/BitMatrix.cpp
    src/BitMatrixIO.h
    src/BitMatrixIO.cpp
    src/ByteArray.h
    src/ByteMatrix.h
    src/CharacterSet.h
    src/CharacterSet.cpp
    src/Content.h
    src/Content.cpp
    src/ECI.h
    src/ECI.cpp
    src/Error.h
    src/Error.cpp
    src/Flags.h
    src/GTIN.h
    src/GTIN.cpp
    src/ImageView.h
    src/Matrix.h
    src/Point.h
    src/Quadrilateral.h
    src/Range.h
    src/ReaderOptions.h
    src/ReadBarcode.h
    src/ReadBarcode.cpp
    src/Utf.h
    src/Utf.cpp
    src/WriteBarcode.h
    src/WriteBarcode.cpp
    src/ZXingCpp.h
    src/ZXingCpp.cpp
    src/ZXAlgorithms.h
    src/ZXConfig.h
    src/ZXTestSupport.h
    src/ZXVersion.h # [[deprecated]]
    $<$<BOOL:${ZXING_C_API}>:src/ZXingC.h>
    $<$<BOOL:${ZXING_C_API}>:src/ZXingC.cpp>
)
if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (COMMON_FILES ${COMMON_FILES}
        src/BitArray.h
        src/BitArray.cpp
        src/Generator.h
        src/GenericGF.h
        src/GenericGF.cpp
        src/GenericGFPoly.h
        src/GenericGFPoly.cpp
        src/TextUtfEncoding.h # [[deprecated]]
        src/TextUtfEncoding.cpp # [[deprecated]]
        src/Scope.h
    )
endif()
if (ZXING_READERS)
    set (COMMON_FILES ${COMMON_FILES}
        src/BinaryBitmap.h
        src/BinaryBitmap.cpp
        src/BitMatrixCursor.h
        src/BitSource.h
        src/BitSource.cpp
        src/ConcentricFinder.h
        src/ConcentricFinder.cpp
        src/DecodeHints.h
        $<$<BOOL:${BUILD_SHARED_LIBS}>:src/DecodeHints.cpp> # [[deprecated]]
        src/DecoderResult.h
        src/DetectorResult.h
        src/GlobalHistogramBinarizer.h
        src/GlobalHistogramBinarizer.cpp
        src/GridSampler.h
        src/GridSampler.cpp
        src/LogMatrix.h
        src/HRI.h
        src/HRI.cpp
        src/HybridBinarizer.h
        src/HybridBinarizer.cpp
        src/MultiFormatReader.h
        src/MultiFormatReader.cpp
        src/Pattern.h
        src/PerspectiveTransform.h
        src/PerspectiveTransform.cpp
        src/Reader.h
        src/ReedSolomonDecoder.h
        src/ReedSolomonDecoder.cpp
        src/RegressionLine.h
        src/Result.h # [[deprecated]]
        src/ResultPoint.h
        src/ResultPoint.cpp
        src/StructuredAppend.h
        src/TextDecoder.h
        src/TextDecoder.cpp
        src/ThresholdBinarizer.h
        src/TritMatrix.h # QRCode
        src/WhiteRectDetector.h
        src/WhiteRectDetector.cpp
    )
endif()

if (ZXING_WRITERS)
    set (COMMON_FILES ${COMMON_FILES}
    )
endif()

if (ZXING_WRITERS_OLD)
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

# define subset of public headers that get distributed with the binaries
set (PUBLIC_HEADERS
    src/Barcode.h
    src/BarcodeFormat.h
    src/BitHacks.h
    src/ByteArray.h
    src/CharacterSet.h
    src/Content.h
    src/Error.h
    src/Flags.h
    src/GTIN.h
    src/ImageView.h
    src/Point.h
    src/Quadrilateral.h
    src/ReadBarcode.h
    src/ReaderOptions.h
    src/StructuredAppend.h
    src/TextUtfEncoding.h # [[deprecated]]
    src/ZXingCpp.h
    src/ZXAlgorithms.h
    src/ZXVersion.h # [[deprecated]]
    $<$<BOOL:${ZXING_C_API}>:${CMAKE_CURRENT_SOURCE_DIR}/src/ZXingC.h>
    $<$<BOOL:${ZXING_EXPERIMENTAL_API}>:${CMAKE_CURRENT_SOURCE_DIR}/src/WriteBarcode.h>
)
if (ZXING_READERS)
    set (PUBLIC_HEADERS ${PUBLIC_HEADERS}
        src/DecodeHints.h # [[deprecated]]
        src/Result.h # [[deprecated]]
    )
endif()
if (ZXING_WRITERS_OLD)
    set (PUBLIC_HEADERS ${PUBLIC_HEADERS}
        src/BitMatrix.h
        src/BitMatrixIO.h
        src/Range.h
        src/Matrix.h
        src/MultiFormatWriter.h
    )
endif()
# end of public header set

if (ZXING_READERS)
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
if (ZXING_WRITERS_OLD)
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


if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (DATAMATRIX_FILES
        src/datamatrix/DMBitLayout.h
        src/datamatrix/DMBitLayout.cpp
        src/datamatrix/DMVersion.h
        src/datamatrix/DMVersion.cpp
    )
endif()
if (ZXING_READERS)
    set (DATAMATRIX_FILES ${DATAMATRIX_FILES}
        src/datamatrix/DMDataBlock.h
        src/datamatrix/DMDataBlock.cpp
        src/datamatrix/DMDecoder.h
        src/datamatrix/DMDecoder.cpp
        src/datamatrix/DMDetector.h
        src/datamatrix/DMDetector.cpp
        src/datamatrix/DMReader.h
        src/datamatrix/DMReader.cpp
    )
endif()
if (ZXING_WRITERS_OLD)
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


if (ZXING_READERS)
    set (MAXICODE_FILES ${MAXICODE_FILES}
        src/maxicode/MCBitMatrixParser.h
        src/maxicode/MCBitMatrixParser.cpp
        src/maxicode/MCDecoder.h
        src/maxicode/MCDecoder.cpp
        src/maxicode/MCReader.h
        src/maxicode/MCReader.cpp
    )
endif()


if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (ONED_FILES
        src/oned/ODUPCEANCommon.h
        src/oned/ODUPCEANCommon.cpp
        src/oned/ODCode128Patterns.h
        src/oned/ODCode128Patterns.cpp
    )
endif()
if (ZXING_READERS)
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
        src/oned/ODDataBarExpandedBitDecoder.h
        src/oned/ODDataBarExpandedBitDecoder.cpp
        src/oned/ODDataBarExpandedReader.h
        src/oned/ODDataBarExpandedReader.cpp
        src/oned/ODDataBarLimitedReader.h
        src/oned/ODDataBarLimitedReader.cpp
        src/oned/ODDXFilmEdgeReader.h
        src/oned/ODDXFilmEdgeReader.cpp
        src/oned/ODITFReader.h
        src/oned/ODITFReader.cpp
        src/oned/ODMultiUPCEANReader.h
        src/oned/ODMultiUPCEANReader.cpp
        src/oned/ODReader.h
        src/oned/ODReader.cpp
        src/oned/ODRowReader.h
    )
endif()
if (ZXING_WRITERS_OLD)
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


if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (PDF417_FILES
        src/pdf417/ZXBigInteger.h
        src/pdf417/ZXBigInteger.cpp
    )
endif()
if (ZXING_READERS)
    set (PDF417_FILES ${PDF417_FILES}
        src/pdf417/PDFBarcodeMetadata.h
        src/pdf417/PDFBarcodeValue.h
        src/pdf417/PDFBarcodeValue.cpp
        src/pdf417/PDFBoundingBox.h
        src/pdf417/PDFBoundingBox.cpp
        src/pdf417/PDFCodeword.h
        src/pdf417/PDFCodewordDecoder.h
        src/pdf417/PDFCodewordDecoder.cpp
        src/pdf417/PDFDecoder.h
        src/pdf417/PDFDecoder.cpp
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
        src/pdf417/CustomData.h
        src/pdf417/ZXNullable.h
    )
endif()
if (ZXING_WRITERS_OLD)
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


if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (QRCODE_FILES
        src/qrcode/QRCodecMode.h
        src/qrcode/QRCodecMode.cpp
        src/qrcode/QRErrorCorrectionLevel.h
        src/qrcode/QRErrorCorrectionLevel.cpp
        src/qrcode/QRVersion.h
        src/qrcode/QRVersion.cpp
    )
endif()
if (ZXING_READERS)
    set (QRCODE_FILES ${QRCODE_FILES}
        src/qrcode/QRBitMatrixParser.h
        src/qrcode/QRBitMatrixParser.cpp
        src/qrcode/QRDataBlock.h
        src/qrcode/QRDataBlock.cpp
        src/qrcode/QRDataMask.h
        src/qrcode/QRDecoder.h
        src/qrcode/QRDecoder.cpp
        src/qrcode/QRDetector.h
        src/qrcode/QRDetector.cpp
        src/qrcode/QRECB.h
        src/qrcode/QRFormatInformation.h
        src/qrcode/QRFormatInformation.cpp
        src/qrcode/QRReader.h
        src/qrcode/QRReader.cpp
    )
endif()
if (ZXING_WRITERS_OLD)
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

source_group (Sources FILES ${COMMON_FILES})
source_group (Sources\\aztec FILES ${AZTEC_FILES})
source_group (Sources\\datamatrix FILES ${DATAMATRIX_FILES})
source_group (Sources\\maxicode FILES ${MAXICODE_FILES})
source_group (Sources\\oned FILES ${ONED_FILES})
source_group (Sources\\pdf417 FILES ${PDF417_FILES})
source_group (Sources\\qrcode FILES ${QRCODE_FILES})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library (ZXing
    ${COMMON_FILES}
    ${AZTEC_FILES}
    ${DATAMATRIX_FILES}
    ${MAXICODE_FILES}
    ${ONED_FILES}
    ${PDF417_FILES}
    ${QRCODE_FILES}
)

target_include_directories (ZXing
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>" "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:include>"
)

target_compile_options (ZXing
    PUBLIC ${ZXING_PUBLIC_FLAGS}
    PRIVATE ${ZXING_PRIVATE_FLAGS}
)

target_compile_features(ZXing PUBLIC cxx_std_17)

target_link_libraries (ZXing PRIVATE Threads::Threads)

if (ZXING_READERS OR ZXING_WRITERS_OLD)
    set (LIBZUECI_FILES
        src/libzueci/zueci.c
        src/libzueci/zueci.h
    )
    set_source_files_properties(${LIBZUECI_FILES} PROPERTIES
        COMPILE_FLAGS "$<$<NOT:$<BOOL:${ZXING_READERS}>>:-DZUECI_EMBED_NO_TO_UTF> $<$<NOT:$<BOOL:${ZXING_WRITERS_OLD}>>:-DZUECI_EMBED_NO_TO_ECI>"
        SKIP_PRECOMPILE_HEADERS ON
    )
    target_sources(ZXing PRIVATE ${LIBZUECI_FILES})
    source_group (Sources\\libzueci FILES ${LIBZUECI_FILES})
endif()

if (ZXING_WRITERS_NEW)
    if (ZXING_USE_BUNDLED_ZINT)
        aux_source_directory(src/libzint LIBZINT_FILES) # manually re-run cmake after adding a new file/symlink
        set_source_files_properties(${LIBZINT_FILES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
        target_sources(ZXing PRIVATE ${LIBZINT_FILES})
        source_group (Sources\\libzint FILES ${LIBZINT_FILES})
        target_include_directories (ZXing PRIVATE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/libzint>")
    else()
        include(../zxing.cmake)
        zxing_add_package(zint zint https://github.com/zint/zint.git 7a9fdd6cd00cd5bfd0082705d934c13ef84f25e1)
        target_link_libraries (ZXing PRIVATE zint)
    endif()
endif()

add_library(ZXing::ZXing ALIAS ZXing)
# add the old alias as well, to keep old clients compiling [[deprecated]]
# note: this only affects client code that includes ZXing via sub_directory.
#       for clients using the exported target, see ZXingConfig.cmake.in
add_library(ZXing::Core ALIAS ZXing)

set_target_properties(ZXing PROPERTIES EXPORT_NAME ZXing)
# force position independent code to be able to link it as static lib into a DLL (e.g. the python module)
set_target_properties(ZXing PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (PROJECT_VERSION)
    set_target_properties(ZXing PROPERTIES VERSION ${PROJECT_VERSION})
    set_target_properties(ZXing PROPERTIES SOVERSION ${ZXING_SONAME})
endif()

set_target_properties(ZXing PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

set(PRECOMPILE_HEADERS ${PUBLIC_HEADERS})
list(REMOVE_ITEM PRECOMPILE_HEADERS "$<$<BOOL:${ZXING_C_API}>:${CMAKE_CURRENT_SOURCE_DIR}/src/ZXingC.h>")
list(REMOVE_ITEM PRECOMPILE_HEADERS src/DecodeHints.h) # [[deprecated]]
list(REMOVE_ITEM PRECOMPILE_HEADERS src/Result.h)      # [[deprecated]]
list(REMOVE_ITEM PRECOMPILE_HEADERS src/ZXVersion.h)   # [[deprecated]]
target_precompile_headers(ZXing PRIVATE ${PRECOMPILE_HEADERS})
set_source_files_properties(src/DecodeHints.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    # The following is a list of translation units that fulfill two criteria regarding the use of -Os vs -O3:
    #  1. their binary size decreases significantly
    #  2. the runtime of ReaderTest is not (measurably) affected
    # Compiling them with -Os saves about 40kB (3%) with clang and 190kB (12%) with gcc.
    check_cxx_compiler_flag("-Os" COMPILER_KNOWS_Os)
    if(COMPILER_KNOWS_Os)
    set_source_files_properties(
        src/Barcode.cpp
        src/BarcodeFormat.cpp
        src/BitMatrixIO.cpp
        src/Error.cpp
        src/GTIN.cpp
        src/HRI.cpp
        src/MultiFormatReader.cpp
        src/WriteBarcode.cpp
        src/ZXingC.cpp
        src/ZXingCpp.cpp
        src/aztec/AZHighLevelEncoder.cpp
        src/datamatrix/DMDataBlock.cpp
        src/datamatrix/DMHighLevelEncoder.cpp
        src/oned/ODDataBarExpandedBitDecoder.cpp
        src/pdf417/PDFHighLevelEncoder.cpp
        src/qrcode/QRBitMatrixParser.cpp
        src/qrcode/QRDataBlock.cpp
        src/qrcode/QRDecoder.cpp
        src/qrcode/QREncoder.cpp
        src/qrcode/QRMaskUtil.cpp
        src/qrcode/QRReader.cpp
        src/qrcode/QRVersion.cpp
        ${LIBZINT_FILES}
        PROPERTIES SKIP_PRECOMPILE_HEADERS ON COMPILE_FLAGS -Os)
    endif()
endif()

include (GNUInstallDirs)

set(ZX_INSTALL_TARGETS ZXing)

install (
    TARGETS ${ZX_INSTALL_TARGETS} EXPORT ZXingTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    INCLUDES DESTINATION include
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ZXing"
)

configure_file (Version.h.in Version.h)

install (
    FILES "${CMAKE_CURRENT_BINARY_DIR}/Version.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ZXing"
)

if (MSVC)
    set_target_properties(ZXing PROPERTIES
        COMPILE_PDB_NAME ZXing
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ZXing.pdb
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            CONFIGURATIONS Debug RelWithDebInfo
            OPTIONAL)
endif()

set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/ZXing")

install (
    EXPORT ZXingTargets
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE ZXing::
)

IF (NOT WIN32 OR MINGW)
    configure_file(zxing.pc.in zxing.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zxing.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
ENDIF()

include (CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file (
    ZXingConfig.cmake.in
    ZXingConfig.cmake
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install (
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ZXingConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)
