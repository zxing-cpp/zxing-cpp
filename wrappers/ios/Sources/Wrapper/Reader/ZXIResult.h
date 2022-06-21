// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIResult : NSObject
@property(nonatomic, strong) NSString *text;
@property(nonatomic, strong) NSData *bytes;
@property(nonatomic) ZXIFormat format;

- (instancetype)init:(NSString *)text
              format:(ZXIFormat)format
               bytes:(NSData *)bytes;
@end

NS_ASSUME_NONNULL_END
