//
//  ZXIPosition.h
//  
//
//  Created by Christian Braun on 21.06.22.
//

#import <Foundation/Foundation.h>
#import "ZXIPoint.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIPosition : NSObject
@property(nonatomic, nonnull)ZXIPoint *topLeft;
@property(nonatomic, nonnull)ZXIPoint *topRight;
@property(nonatomic, nonnull)ZXIPoint *bottomRight;
@property(nonatomic, nonnull)ZXIPoint *bottomLeft;

- (instancetype)initWithTopLeft:(ZXIPoint *)topLeft
                       topRight:(ZXIPoint *)topRight
                    bottomRight:(ZXIPoint *)bottomRight
                     bottomLeft:(ZXIPoint *)bottomLeft;
@end

NS_ASSUME_NONNULL_END
