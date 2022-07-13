// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <CoreGraphics/CoreGraphics.h>
#import <CoreImage/CoreImage.h>
#import "ZXIResult.h"
#import "ZXIDecodeHints.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeReader : NSObject
@property(nonatomic, strong) ZXIDecodeHints *hints;

- (instancetype)initWithHints:(ZXIDecodeHints*)options;
- (NSArray<ZXIResult *> *)readCIImage:(nonnull CIImage *)image;
- (NSArray<ZXIResult *> *)readCGImage:(nonnull CGImageRef)image;
- (NSArray<ZXIResult *> *)readCVPixelBuffer:(nonnull CVPixelBufferRef)pixelBuffer;
@end

NS_ASSUME_NONNULL_END
