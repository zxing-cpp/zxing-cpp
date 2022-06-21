//
//  ZXIPositionHelper.h
//  
//
//  Created by Christian Braun on 21.06.22.
//

#import <Foundation/Foundation.h>
#import "ZXIPosition.h"
#import "ZXing/Result.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIPosition(Helper)
- (instancetype)initWithPosition:(ZXing::Position)position;
@end

NS_ASSUME_NONNULL_END
