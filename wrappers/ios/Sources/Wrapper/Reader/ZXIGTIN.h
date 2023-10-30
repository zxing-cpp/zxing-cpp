// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIGTIN : NSObject
@property(nonatomic, nonnull)NSString *country;
@property(nonatomic, nonnull)NSString *addOn;
@property(nonatomic, nonnull)NSString *price;
@property(nonatomic, nonnull)NSString *issueNumber;

- (instancetype)initWithCountry:(NSString *)country
                          addOn:(NSString *)addOn
                          price:(NSString *)price
                    issueNumber:(NSString *)issueNumber;
@end

NS_ASSUME_NONNULL_END
