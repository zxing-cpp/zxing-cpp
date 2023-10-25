// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIEncodeHints.h"

NS_ASSUME_NONNULL_BEGIN

const int AZTEC_ERROR_CORRECTION_0 = 0;
const int AZTEC_ERROR_CORRECTION_12 = 1;
const int AZTEC_ERROR_CORRECTION_25 = 2;
const int AZTEC_ERROR_CORRECTION_37 = 3;
const int AZTEC_ERROR_CORRECTION_50 = 4;
const int AZTEC_ERROR_CORRECTION_62 = 5;
const int AZTEC_ERROR_CORRECTION_75 = 6;
const int AZTEC_ERROR_CORRECTION_87 = 7;
const int AZTEC_ERROR_CORRECTION_100 = 8;
const int QR_ERROR_CORRECTION_LOW = 2;
const int QR_ERROR_CORRECTION_MEDIUM = 4;
const int QR_ERROR_CORRECTION_QUARTILE = 6;
const int QR_ERROR_CORRECTION_HIGH = 8;
const int PDF417_ERROR_CORRECTION_0 = 0;
const int PDF417_ERROR_CORRECTION_1 = 1;
const int PDF417_ERROR_CORRECTION_2 = 2;
const int PDF417_ERROR_CORRECTION_3 = 3;
const int PDF417_ERROR_CORRECTION_4 = 4;
const int PDF417_ERROR_CORRECTION_5 = 5;
const int PDF417_ERROR_CORRECTION_6 = 6;
const int PDF417_ERROR_CORRECTION_7 = 7;
const int PDF417_ERROR_CORRECTION_8 = 8;

@interface ZXIBarcodeWriter : NSObject

-(nullable CGImageRef)writeString:(NSString *)contents
                            hints:(ZXIEncodeHints *)hints
                            error:(NSError *__autoreleasing  _Nullable *)error;

-(nullable CGImageRef)writeData:(NSData *)data
                          hints:(ZXIEncodeHints *)hints
                          error:(NSError *__autoreleasing  _Nullable *)error;

@end

NS_ASSUME_NONNULL_END
