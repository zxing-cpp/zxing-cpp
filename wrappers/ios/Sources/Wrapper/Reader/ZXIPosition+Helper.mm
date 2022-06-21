// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import "ZXIPosition+Helper.h"
#import "ZXIPoint.h"

@implementation ZXIPosition(Helper)
-(instancetype)initWithPosition:(ZXing::Position)position {
    return [self initWithTopLeft:[[ZXIPoint alloc] initWithX:position.topLeft().x y:position.topLeft().y]
                        topRight:[[ZXIPoint alloc] initWithX:position.topRight().x y:position.topRight().y]
                     bottomRight:[[ZXIPoint alloc] initWithX:position.bottomRight().x y:position.bottomRight().y]
                      bottomLeft:[[ZXIPoint alloc] initWithX:position.bottomLeft().x y:position.bottomLeft().y]];
}

@end
