// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIPoint : NSObject
@property(nonatomic) NSInteger x;
@property(nonatomic) NSInteger y;

- (instancetype)initWithX:(NSInteger)x y:(NSInteger)y;
@end

NS_ASSUME_NONNULL_END
