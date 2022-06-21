// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import "ZXIPosition.h"

@implementation ZXIPosition
- (instancetype)initWithTopLeft:(ZXIPoint *)topLeft
                       topRight:(ZXIPoint *)topRight
                    bottomRight:(ZXIPoint *)bottomRight
                     bottomLeft:(ZXIPoint *)bottomLeft {
    self = [super init];
    self.topLeft = topLeft;
    self.topRight = topRight;
    self.bottomRight = bottomRight;
    self.bottomLeft = bottomLeft;
    return self;
}
@end
