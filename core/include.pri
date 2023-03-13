#common

SOURCES +=  \
        $$PWD/src/BarcodeFormat.cpp \
        $$PWD/src/BitArray.cpp \
        $$PWD/src/BitMatrix.cpp \
        $$PWD/src/BitMatrixIO.cpp \
        $$PWD/src/CharacterSet.cpp \
        $$PWD/src/ConcentricFinder.cpp \
        $$PWD/src/ECI.cpp \
        $$PWD/src/GenericGF.cpp \
        $$PWD/src/GenericGFPoly.h \
        $$PWD/src/GenericGFPoly.cpp \
        $$PWD/src/GTIN.cpp \
        $$PWD/src/TextUtfEncoding.cpp \ # [[deprecated]]
        $$PWD/src/Utf.cpp \
        $$PWD/src/ZXBigInteger.cpp
#        $$PWD/../example/ZXingQtReader.cpp \

HEADERS += \
    $$PWD/../example/ZXingQtReader.h \
    $$PWD/src/BarcodeFormat.h \
    $$PWD/src/BitArray.h \
    $$PWD/src/BitHacks.h \
    $$PWD/src/BitMatrix.h \
    $$PWD/src/BitMatrixCursor.h \
    $$PWD/src/BitMatrixIO.h \
    $$PWD/src/ByteArray.h \
    $$PWD/src/ByteMatrix.h \
    $$PWD/src/CharacterSet.h \
    $$PWD/src/ConcentricFinder.h \
    $$PWD/src/CustomData.h \
    $$PWD/src/ECI.h \
    $$PWD/src/Flags.h \
    $$PWD/src/Generator.h \
    $$PWD/src/GenericGF.h \
    $$PWD/src/GTIN.h \
    $$PWD/src/LogMatrix.h \
    $$PWD/src/Matrix.h \
    $$PWD/src/Pattern.h \
    $$PWD/src/Point.h \
    $$PWD/src/Quadrilateral.h \
    $$PWD/src/Range.h \
    $$PWD/src/RegressionLine.h \
    $$PWD/src/Scope.h \
    $$PWD/src/TextUtfEncoding.h \ # [[deprecated]]
    $$PWD/src/Utf.h \
    $$PWD/src/TritMatrix.h \
    $$PWD/src/ZXAlgorithms.h \
    $$PWD/src/ZXBigInteger.h \
    $$PWD/src/ZXConfig.h \
    $$PWD/src/ZXNullable.h \
    $$PWD/src/ZXTestSupport.h

#READER
SOURCES +=  \
    $$PWD/src/libzueci/zueci.c \
    $$PWD/src/BinaryBitmap.cpp \
    $$PWD/src/BitSource.cpp \
    $$PWD/src/Content.cpp \
    $$PWD/src/DecodeHints.cpp \
    $$PWD/src/GlobalHistogramBinarizer.cpp \
    $$PWD/src/GridSampler.cpp \
    $$PWD/src/HybridBinarizer.cpp \
    $$PWD/src/PerspectiveTransform.cpp \
    $$PWD/src/Result.cpp \
    $$PWD/src/ReadBarcode.cpp \
    $$PWD/src/ResultPoint.cpp \
    $$PWD/src/HRI.cpp \
    $$PWD/src/aztec/AZDecoder.cpp \
    $$PWD/src/aztec/AZDetector.cpp \
    $$PWD/src/aztec/AZReader.cpp \
    $$PWD/src/datamatrix/DMDecoder.cpp \
    $$PWD/src/datamatrix/DMDetector.cpp \
    $$PWD/src/datamatrix/DMReader.cpp \
    $$PWD/src/maxicode/MCDecoder.cpp \
    $$PWD/src/maxicode/MCReader.cpp \
    $$PWD/src/oned/ODCodabarReader.cpp \
    $$PWD/src/oned/ODCode128Reader.cpp \
    $$PWD/src/oned/ODCode39Reader.cpp \
    $$PWD/src/oned/ODCode93Reader.cpp \
    $$PWD/src/oned/ODDataBarExpandedBitDecoder.cpp \
    $$PWD/src/oned/ODDataBarExpandedReader.cpp \
    $$PWD/src/oned/ODDataBarReader.cpp \
    $$PWD/src/oned/ODITFReader.cpp \
    $$PWD/src/oned/ODMultiUPCEANReader.cpp \
    $$PWD/src/oned/ODReader.cpp \
    $$PWD/src/oned/ODRowReader.cpp \
    $$PWD/src/pdf417/PDFCodewordDecoder.cpp \
    $$PWD/src/pdf417/PDFDetector.cpp \
    $$PWD/src/pdf417/PDFReader.cpp \
    $$PWD/src/pdf417/PDFScanningDecoder.cpp \
    $$PWD/src/qrcode/QRDecoder.cpp \
    $$PWD/src/qrcode/QRDetector.cpp \
    $$PWD/src/qrcode/QRReader.cpp \
    $$PWD/src/MultiFormatReader.cpp \
    $$PWD/src/ReedSolomonDecoder.cpp \
    $$PWD/src/TextDecoder.cpp \
    $$PWD/src/WhiteRectDetector.cpp \
    $$PWD/src/oned/ODCode128Patterns.cpp \
    $$PWD/src/datamatrix/DMBitLayout.cpp \
    $$PWD/src/datamatrix/DMDataBlock.cpp \
    $$PWD/src/datamatrix/DMVersion.cpp \
    $$PWD/src/oned/ODDataBarCommon.cpp \
    $$PWD/src/oned/ODUPCEANCommon.cpp \
    $$PWD/src/maxicode/MCBitMatrixParser.cpp \
    $$PWD/src/pdf417/PDFDecodedBitStreamParser.cpp \
    $$PWD/src/qrcode/QRBitMatrixParser.cpp \
    $$PWD/src/qrcode/QRDataBlock.cpp \
    $$PWD/src/qrcode/QRCodecMode.cpp \
    $$PWD/src/qrcode/QRVersion.cpp \
    $$PWD/src/qrcode/QRFormatInformation.cpp \
    $$PWD/src/qrcode/QRErrorCorrectionLevel.cpp \
    $$PWD/src/pdf417/PDFBoundingBox.cpp \
    $$PWD/src/pdf417/PDFDetectionResult.cpp \
    $$PWD/src/pdf417/PDFDetectionResultColumn.cpp \
    $$PWD/src/pdf417/PDFBarcodeValue.cpp \
    $$PWD/src/pdf417/PDFModulusPoly.cpp \
    $$PWD/src/pdf417/PDFModulusGF.cpp



HEADERS += \
    $$PWD/src/libzueci/zueci.h \
    $$PWD/src/BinaryBitmap.h \
    $$PWD/src/BitSource.h \
    $$PWD/src/Content.h \
    $$PWD/src/DecodeHints.h \
    $$PWD/src/DecoderResult.h \
    $$PWD/src/DetectorResult.h \
    $$PWD/src/Error.h \
    $$PWD/src/GlobalHistogramBinarizer.h \
    $$PWD/src/GridSampler.h \
    $$PWD/src/HRI.h \
    $$PWD/src/HybridBinarizer.h \
    $$PWD/src/ImageView.h \
    $$PWD/src/PerspectiveTransform.h \
    $$PWD/src/Reader.h \
    $$PWD/src/ReadBarcode.h \
    $$PWD/src/Result.h \
    $$PWD/src/ResultPoint.h \
    $$PWD/src/StructuredAppend.h \
    $$PWD/src/ThresholdBinarizer.h \
    $$PWD/src/aztec/AZDecoder.h \
    $$PWD/src/aztec/AZDetector.h \
    $$PWD/src/aztec/AZReader.h \
    $$PWD/src/datamatrix/DMDecoder.h \
    $$PWD/src/datamatrix/DMDetector.h \
    $$PWD/src/datamatrix/DMReader.h \
    $$PWD/src/maxicode/MCDecoder.h \
    $$PWD/src/maxicode/MCReader.h \
    $$PWD/src/oned/ODCodabarReader.h \
    $$PWD/src/oned/ODCode128Reader.h \
    $$PWD/src/oned/ODCode39Reader.h \
    $$PWD/src/oned/ODCode93Reader.h \
    $$PWD/src/oned/ODDataBarExpandedBitDecoder.h \
    $$PWD/src/oned/ODDataBarExpandedReader.h \
    $$PWD/src/oned/ODDataBarReader.h \
    $$PWD/src/oned/ODITFReader.h \
    $$PWD/src/oned/ODMultiUPCEANReader.h \
    $$PWD/src/oned/ODReader.h \
    $$PWD/src/oned/ODRowReader.h \
    $$PWD/src/pdf417/PDFCodewordDecoder.h \
    $$PWD/src/pdf417/PDFDetector.h \
    $$PWD/src/pdf417/PDFReader.h \
    $$PWD/src/pdf417/PDFScanningDecoder.h \
    $$PWD/src/qrcode/QRDecoder.h \
    $$PWD/src/qrcode/QRDetector.h \
    $$PWD/src/qrcode/QRReader.h \
    $$PWD/src/MultiFormatReader.h \
    $$PWD/src/Reader.h \
    $$PWD/src/ReedSolomonDecoder.h \
    $$PWD/src/WhiteRectDetector.h \
    $$PWD/src/oned/ODCode128Patterns.h \
    $$PWD/src/datamatrix/DMBitLayout.h \
    $$PWD/src/datamatrix/DMDataBlock.h \
    $$PWD/src/datamatrix/DMVersion.h \
    $$PWD/src/oned/ODDataBarCommon.h \
    $$PWD/src/oned/ODUPCEANCommon.h \
    $$PWD/src/maxicode/MCBitMatrixParser.h \
    $$PWD/src/pdf417/PDFDecodedBitStreamParser.h \
    $$PWD/src/qrcode/QRBitMatrixParser.h \
    $$PWD/src/qrcode/QRDataBlock.h \
    $$PWD/src/qrcode/QRCodecMode.h \
    $$PWD/src/qrcode/QRVersion.h \
    $$PWD/src/qrcode/QRFormatInformation.h \
    $$PWD/src/qrcode/QRErrorCorrectionLevel.h \
    $$PWD/src/pdf417/PDFBoundingBox.h \
    $$PWD/src/pdf417/PDFDetectionResult.h \
    $$PWD/src/pdf417/PDFDetectionResultColumn.h \
    $$PWD/src/pdf417/PDFBarcodeValue.h \
    $$PWD/src/pdf417/PDFModulusPoly.h \
    $$PWD/src/pdf417/PDFModulusGF.h

