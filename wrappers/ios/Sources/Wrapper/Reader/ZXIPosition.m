//
//  ZXIPosition.m
//  
//
//  Created by Christian Braun on 21.06.22.
//

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
