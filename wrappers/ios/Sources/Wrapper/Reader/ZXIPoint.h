//
//  ZXIPoint.h
//  
//
//  Created by Christian Braun on 21.06.22.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIPoint : NSObject
@property(nonatomic) NSInteger x;
@property(nonatomic) NSInteger y;

- (instancetype)initWithX:(NSInteger)x y:(NSInteger)y;
@end

NS_ASSUME_NONNULL_END
