// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0


#import "ZXIGTIN.h"

@implementation ZXIGTIN
- (instancetype)initWithCountry:(NSString *)country
                          addOn:(NSString *)addOn
                          price:(NSString *)price
                    issueNumber:(NSString *)issueNumber {
    self = [super init];
    self.country = country;
    self.addOn = addOn;
    self.price = price;
    self.issueNumber = issueNumber;
    return self;
}
@end
