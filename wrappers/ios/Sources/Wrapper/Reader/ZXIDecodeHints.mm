//
//  ZXIDecodeHints.m
//  
//
//  Created by Christian Braun on 25.02.22.
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIDecodeHints.h"

@implementation ZXIDecodeHints

- (instancetype)initWithTryHarder:(BOOL)tryHarder tryRotate:(BOOL)tryRotate formats:(NSArray<NSNumber*>*)formats {
    self = [super init];
    self.tryHarder = tryHarder;
    self.tryRotate = tryRotate;
    self.formats = formats;
    return self;
}

@end
