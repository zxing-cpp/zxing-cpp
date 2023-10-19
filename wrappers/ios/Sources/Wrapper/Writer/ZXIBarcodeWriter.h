// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeWriter : NSObject

-(nullable CGImageRef)writeText:(NSString *)contents
                          width:(int)width
                         height:(int)height
                         format:(ZXIFormat)format
                          error:(NSError *__autoreleasing  _Nullable *)error;

-(nullable CGImageRef)writeBytes:(NSData *)data
                           width:(int)width
                          height:(int)height
                          format:(ZXIFormat)format
                           error:(NSError *__autoreleasing  _Nullable *)error;

@end

NS_ASSUME_NONNULL_END
