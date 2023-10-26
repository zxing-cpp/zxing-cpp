// Copyright 2023 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIEncodeHints : NSObject
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
