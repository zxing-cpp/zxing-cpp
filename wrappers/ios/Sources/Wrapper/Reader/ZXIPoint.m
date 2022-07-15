// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import "ZXIPoint.h"

@implementation ZXIPoint

- (instancetype)initWithX:(NSInteger)x y:(NSInteger)y {
    self = [super init];
    self.x = x;
    self.y = y;
    return self;
}
@end
