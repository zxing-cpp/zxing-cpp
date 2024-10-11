// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIFormatHelper.h"

ZXing::BarcodeFormat BarcodeFormatFromZXIFormat(ZXIFormat format) {
    switch (format) {
        case ZXIFormat::ANY:
            return ZXing::BarcodeFormat::Any;
        case ZXIFormat::MATRIX_CODES:
            return ZXing::BarcodeFormat::MatrixCodes;
        case ZXIFormat::LINEAR_CODES:
            return ZXing::BarcodeFormat::LinearCodes;
        case ZXIFormat::UPC_E:
            return ZXing::BarcodeFormat::UPCE;
        case ZXIFormat::UPC_A:
            return ZXing::BarcodeFormat::UPCA;
        case ZXIFormat::QR_CODE:
            return ZXing::BarcodeFormat::QRCode;
        case ZXIFormat::PDF_417:
            return ZXing::BarcodeFormat::PDF417;
        case ZXIFormat::MAXICODE:
            return ZXing::BarcodeFormat::MaxiCode;
        case ZXIFormat::ITF:
            return ZXing::BarcodeFormat::ITF;
        case ZXIFormat::EAN_13:
            return ZXing::BarcodeFormat::EAN13;
        case ZXIFormat::EAN_8:
            return ZXing::BarcodeFormat::EAN8;
        case ZXIFormat::DATA_MATRIX:
            return ZXing::BarcodeFormat::DataMatrix;
        case ZXIFormat::DATA_BAR_EXPANDED:
            return ZXing::BarcodeFormat::DataBarExpanded;
        case ZXIFormat::DATA_BAR_LIMITED:
            return ZXing::BarcodeFormat::DataBarLimited;
        case ZXIFormat::DATA_BAR:
            return ZXing::BarcodeFormat::DataBar;
        case ZXIFormat::DX_FILM_EDGE:
            return ZXing::BarcodeFormat::DXFilmEdge;
        case ZXIFormat::CODE_128:
            return ZXing::BarcodeFormat::Code128;
        case ZXIFormat::CODE_93:
            return ZXing::BarcodeFormat::Code93;
        case ZXIFormat::CODE_39:
            return ZXing::BarcodeFormat::Code39;
        case ZXIFormat::CODABAR:
            return ZXing::BarcodeFormat::Codabar;
        case ZXIFormat::AZTEC:
            return ZXing::BarcodeFormat::Aztec;
        case ZXIFormat::MICRO_QR_CODE:
            return ZXing::BarcodeFormat::MicroQRCode;
        case ZXIFormat::RMQR_CODE:
            return ZXing::BarcodeFormat::RMQRCode;
        case ZXIFormat::NONE:
            return ZXing::BarcodeFormat::None;
    }
    NSLog(@"ZXIWrapper: Received invalid ZXIFormat, returning format: None");
    return ZXing::BarcodeFormat::None;
}

ZXIFormat ZXIFormatFromBarcodeFormat(ZXing::BarcodeFormat format) {
    switch (format) {
        case ZXing::BarcodeFormat::None:
            return ZXIFormat::NONE;
        case ZXing::BarcodeFormat::Aztec:
            return ZXIFormat::AZTEC;
        case ZXing::BarcodeFormat::Codabar:
            return ZXIFormat::CODABAR;
        case ZXing::BarcodeFormat::Code39:
            return ZXIFormat::CODE_39;
        case ZXing::BarcodeFormat::Code93:
            return ZXIFormat::CODE_93;
        case ZXing::BarcodeFormat::Code128:
            return ZXIFormat::CODE_128;
        case ZXing::BarcodeFormat::DataBar:
            return ZXIFormat::DATA_BAR;
        case ZXing::BarcodeFormat::DataBarExpanded:
            return ZXIFormat::DATA_BAR_EXPANDED;
        case ZXing::BarcodeFormat::DataBarLimited:
            return ZXIFormat::DATA_BAR_LIMITED;
        case ZXing::BarcodeFormat::DataMatrix:
            return ZXIFormat::DATA_MATRIX;
        case ZXing::BarcodeFormat::DXFilmEdge:
            return ZXIFormat::DX_FILM_EDGE;
        case ZXing::BarcodeFormat::EAN8:
            return ZXIFormat::EAN_8;
        case ZXing::BarcodeFormat::EAN13:
            return ZXIFormat::EAN_13;
        case ZXing::BarcodeFormat::ITF:
            return ZXIFormat::ITF;
        case ZXing::BarcodeFormat::MaxiCode:
            return ZXIFormat::MAXICODE;
        case ZXing::BarcodeFormat::PDF417:
            return ZXIFormat::PDF_417;
        case ZXing::BarcodeFormat::QRCode:
            return ZXIFormat::QR_CODE;
        case ZXing::BarcodeFormat::UPCA:
            return ZXIFormat::UPC_A;
        case ZXing::BarcodeFormat::UPCE:
            return ZXIFormat::UPC_E;
        case ZXing::BarcodeFormat::LinearCodes:
            return ZXIFormat::LINEAR_CODES;
        case ZXing::BarcodeFormat::MatrixCodes:
            return ZXIFormat::MATRIX_CODES;
        case ZXing::BarcodeFormat::MicroQRCode:
            return ZXIFormat::MICRO_QR_CODE;
        case ZXing::BarcodeFormat::RMQRCode:
            return ZXIFormat::RMQR_CODE;
        case ZXing::BarcodeFormat::Any:
            return ZXIFormat::ANY;
    }
    NSLog(@"ZXIWrapper: Received invalid BarcodeFormat, returning format: None");
    return ZXIFormat::NONE;
}
