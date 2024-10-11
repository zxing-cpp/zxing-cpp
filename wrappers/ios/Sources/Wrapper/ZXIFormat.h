// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#ifndef ZXIFormat_h
#define ZXIFormat_h

typedef NS_ENUM(NSInteger, ZXIFormat) {
    NONE, AZTEC, CODABAR, CODE_39, CODE_93, CODE_128, DATA_BAR, DATA_BAR_EXPANDED, DATA_BAR_LIMITED,
    DATA_MATRIX, DX_FILM_EDGE, EAN_8, EAN_13, ITF, MAXICODE, PDF_417, QR_CODE, MICRO_QR_CODE, RMQR_CODE, UPC_A, UPC_E,
    LINEAR_CODES, MATRIX_CODES, ANY
};

#endif 
