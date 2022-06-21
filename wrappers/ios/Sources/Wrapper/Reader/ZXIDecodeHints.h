// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIDecodeHints : NSObject
@property(nonatomic) BOOL tryHarder;
@property(nonatomic) BOOL tryRotate;
@property(nonatomic) BOOL tryDownscale;
@property(nonatomic) NSInteger maxNumberOfSymbols;
/// An array of ZXIFormat
@property(nonatomic, strong) NSArray<NSNumber*> *formats;

- (instancetype)initWithTryHarder:(BOOL)tryHarder
                        tryRotate:(BOOL)tryRotate
                     tryDownscale:(BOOL)tryDownscale
               maxNumberOfSymbols:(NSInteger)maxNumberOfSymbol
                          formats:(NSArray<NSNumber*>*)formats;
@end

NS_ASSUME_NONNULL_END
