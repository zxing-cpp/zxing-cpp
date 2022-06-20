// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIDecodeHints.h"

@implementation ZXIDecodeHints

- (instancetype)initWithTryHarder:(BOOL)tryHarder
                        tryRotate:(BOOL)tryRotate
                     tryDownscale:(BOOL)tryDownscale
               maxNumberOfSymbols:(NSInteger)maxNumberOfSymbols
                          formats:(NSArray<NSNumber*>*)formats {
    self = [super init];
    self.tryHarder = tryHarder;
    self.tryRotate = tryRotate;
    self.tryDownscale = tryDownscale;
    self.maxNumberOfSymbols = maxNumberOfSymbols;
    self.formats = formats;
    return self;
}

@end
