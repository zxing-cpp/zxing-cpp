// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIPosition.h"
#import "Barcode.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIPosition(Helper)
- (instancetype)initWithPosition:(ZXing::Position)position;
@end

NS_ASSUME_NONNULL_END
