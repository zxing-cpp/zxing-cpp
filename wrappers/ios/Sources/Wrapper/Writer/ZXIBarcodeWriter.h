// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIWriterOptions.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeWriter : NSObject
@property(nonatomic, strong) ZXIWriterOptions *options;

-(instancetype)initWithOptions:(ZXIWriterOptions*)options;

-(nullable CGImageRef)writeString:(NSString *)contents
                            error:(NSError *__autoreleasing  _Nullable *)error;

-(nullable CGImageRef)writeData:(NSData *)data
                          error:(NSError *__autoreleasing  _Nullable *)error;

@end

NS_ASSUME_NONNULL_END
