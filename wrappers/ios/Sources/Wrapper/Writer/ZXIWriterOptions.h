// Copyright 2023 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

extern const int AZTEC_ERROR_CORRECTION_0;
extern const int AZTEC_ERROR_CORRECTION_12;
extern const int AZTEC_ERROR_CORRECTION_25;
extern const int AZTEC_ERROR_CORRECTION_37;
extern const int AZTEC_ERROR_CORRECTION_50;
extern const int AZTEC_ERROR_CORRECTION_62;
extern const int AZTEC_ERROR_CORRECTION_75;
extern const int AZTEC_ERROR_CORRECTION_87;
extern const int AZTEC_ERROR_CORRECTION_100;
extern const int QR_ERROR_CORRECTION_LOW;
extern const int QR_ERROR_CORRECTION_MEDIUM;
extern const int QR_ERROR_CORRECTION_QUARTILE;
extern const int QR_ERROR_CORRECTION_HIGH;
extern const int PDF417_ERROR_CORRECTION_0;
extern const int PDF417_ERROR_CORRECTION_1;
extern const int PDF417_ERROR_CORRECTION_2;
extern const int PDF417_ERROR_CORRECTION_3;
extern const int PDF417_ERROR_CORRECTION_4;
extern const int PDF417_ERROR_CORRECTION_5;
extern const int PDF417_ERROR_CORRECTION_6;
extern const int PDF417_ERROR_CORRECTION_7;
extern const int PDF417_ERROR_CORRECTION_8;

@interface ZXIWriterOptions : NSObject
@property(nonatomic) ZXIFormat format;
@property(nonatomic) int width;
@property(nonatomic) int height;
@property(nonatomic) int ecLevel;
@property(nonatomic) int margin;

- (instancetype)initWithFormat:(ZXIFormat)format;

- (instancetype)initWithFormat:(ZXIFormat)format
                         width:(int)width
                        height:(int)height
                       ecLevel:(int)ecLevel
                        margin:(int)margin;
@end

NS_ASSUME_NONNULL_END
