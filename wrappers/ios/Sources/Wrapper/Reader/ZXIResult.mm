// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIResult.h"

@implementation ZXIResult
- (instancetype)init:(NSString *)text
              format:(ZXIFormat)format
               bytes:(NSData *)bytes {
    self = [super init];
    self.text = text;
    self.format = format;
    self.bytes = bytes;
    return self;
}
@end
