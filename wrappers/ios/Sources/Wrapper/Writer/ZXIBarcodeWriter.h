// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIEncodeHints.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeWriter : NSObject

-(nullable CGImageRef)writeString:(NSString *)contents
                            hints:(ZXIEncodeHints *)hints
                            error:(NSError *__autoreleasing  _Nullable *)error;

-(nullable CGImageRef)writeData:(NSData *)data
                          hints:(ZXIEncodeHints *)hints
                          error:(NSError *__autoreleasing  _Nullable *)error;

@end

NS_ASSUME_NONNULL_END
