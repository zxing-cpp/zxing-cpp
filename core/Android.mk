LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := zxing_core

LOCAL_CPPFLAGS += -std=c++11
LOCAL_CFLAGS += -Wall
LOCAL_CPPFLAGS += -Wno-missing-braces  # Seems to be a bug in clang https://llvm.org/bugs/show_bug.cgi?id=21629

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/src

COMMON_FILES :=	\
	src/BarcodeFormat.cpp \
	src/BitArray.cpp \
	src/BitMatrix.cpp \
	src/BitSource.cpp \
	src/CharacterSetECI.cpp \
	src/DecodeHints.cpp \
	src/DecodeStatus.cpp \
	src/GenericGF.cpp \
	src/GenericGFPoly.cpp \
	src/GenericLuminanceSource.cpp \
	src/GlobalHistogramBinarizer.cpp \
	src/GridSampler.cpp \
	src/HybridBinarizer.cpp \
	src/LuminanceSource.cpp \
	src/MultiFormatReader.cpp \
	src/PerspectiveTransform.cpp \
	src/ReedSolomonDecoder.cpp \
	src/Result.cpp \
	src/ResultMetadata.cpp \
	src/ResultPoint.cpp \
	src/TextDecoder.cpp \
	src/TextUtfEncoding.cpp \
	src/WhiteRectDetector.cpp \
	src/ZXBigInteger.cpp

AZTEC_FILES := \
	src/aztec/AZDecoder.cpp \
	src/aztec/AZDetector.cpp \
	src/aztec/AZReader.cpp

DATAMATRIX_FILES :=	\
	src/datamatrix/DMBitMatrixParser.cpp \
	src/datamatrix/DMDataBlock.cpp \
	src/datamatrix/DMDecoder.cpp \
	src/datamatrix/DMDetector.cpp \
	src/datamatrix/DMReader.cpp \
	src/datamatrix/DMVersion.cpp

MAXICODE_FILES := \
	src/maxicode/MCBitMatrixParser.cpp \
	src/maxicode/MCDecoder.cpp \
	src/maxicode/MCReader.cpp
	
ONED_FILES := \
	src/oned/ODCodabarReader.cpp \
	src/oned/ODCode128Patterns.cpp \
	src/oned/ODCode128Reader.cpp \
	src/oned/ODCode39Reader.cpp \
	src/oned/ODCode93Reader.cpp \
	src/oned/ODEAN13Reader.cpp \
	src/oned/ODEAN8Reader.cpp \
	src/oned/ODEANManufacturerOrgSupport.cpp \
	src/oned/ODITFReader.cpp \
	src/oned/ODMultiUPCEANReader.cpp \
	src/oned/ODRSS14Reader.cpp \
	src/oned/ODRSSExpandedReader.cpp \
	src/oned/ODReader.cpp \
	src/oned/ODRowReader.cpp \
	src/oned/ODUPCAReader.cpp \
    src/oned/ODUPCEANCommon.cpp \
	src/oned/ODUPCEANExtensionSupport.cpp \
	src/oned/ODUPCEANReader.cpp \
	src/oned/ODUPCEReader.cpp

ONED_RSS_FILES := \
	src/oned/rss/ODRSSExpandedBinaryDecoder.cpp \
	src/oned/rss/ODRSSFieldParser.cpp \
	src/oned/rss/ODRSSGenericAppIdDecoder.cpp \
	src/oned/rss/ODRSSReaderHelper.cpp

PDF417_FILES := \
	src/pdf417/PDFBarcodeValue.cpp \
	src/pdf417/PDFBoundingBox.cpp \
	src/pdf417/PDFCodewordDecoder.cpp \
	src/pdf417/PDFDecodedBitStreamParser.cpp \
	src/pdf417/PDFDetectionResult.cpp \
	src/pdf417/PDFDetectionResultColumn.cpp \
	src/pdf417/PDFDetector.cpp \
	src/pdf417/PDFModulusGF.cpp \
	src/pdf417/PDFModulusPoly.cpp \
	src/pdf417/PDFReader.cpp \
	src/pdf417/PDFScanningDecoder.cpp

QRCODE_FILES := \
	src/qrcode/QRAlignmentPattern.cpp \
	src/qrcode/QRAlignmentPatternFinder.cpp \
	src/qrcode/QRBitMatrixParser.cpp \
	src/qrcode/QRCodecMode.cpp \
	src/qrcode/QRDataBlock.cpp \
	src/qrcode/QRDataMask.cpp \
	src/qrcode/QRDecoder.cpp \
	src/qrcode/QRDetector.cpp \
	src/qrcode/QRErrorCorrectionLevel.cpp \
	src/qrcode/QRFinderPattern.cpp \
	src/qrcode/QRFinderPatternFinder.cpp \
	src/qrcode/QRFormatInformation.cpp \
	src/qrcode/QRReader.cpp \
	src/qrcode/QRVersion.cpp

TEXT_CODEC_FILES := \
	src/textcodec/Big5MapTable.cpp \
	src/textcodec/Big5TextDecoder.cpp \
	src/textcodec/GBTextDecoder.cpp \
	src/textcodec/JPTextDecoder.cpp \
	src/textcodec/KRHangulMapping.cpp \
	src/textcodec/KRTextDecoder.cpp

LOCAL_SRC_FILES := \
	$(COMMON_FILES) \
	$(AZTEC_FILES) \
	$(DATAMATRIX_FILES) \
	$(MAXICODE_FILES) \
	$(ONED_FILES) \
	$(ONED_RSS_FILES) \
	$(PDF417_FILES) \
	$(QRCODE_FILES) \
	$(TEXT_CODEC_FILES)

LOCAL_CPP_FEATURES += rtti exceptions

include $(BUILD_STATIC_LIBRARY)
