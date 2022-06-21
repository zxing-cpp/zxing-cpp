//
//  ZXIPoint.m
//  
//
//  Created by Christian Braun on 21.06.22.
//

#import "ZXIPoint.h"

@implementation ZXIPoint

- (instancetype)initWithX:(NSInteger)x y:(NSInteger)y {
    self = [super init];
    self.x = x;
    self.y = y;
    return self;
}
@end
