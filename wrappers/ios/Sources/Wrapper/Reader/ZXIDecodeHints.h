// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIDecodeHints : NSObject
@property(nonatomic) BOOL tryHarder;
@property(nonatomic) BOOL tryRotate;
@property(nonatomic) BOOL tryDownscale;
@property(nonatomic) BOOL tryInvert;
@property(nonatomic) BOOL tryCode39ExtendedMode;
@property(nonatomic) BOOL validateCode39CheckSum;
@property(nonatomic) BOOL validateITFCheckSum;
@property(nonatomic) uint8_t downscaleFactor;
@property(nonatomic) uint16_t downscaleThreshold;

@property(nonatomic) NSInteger maxNumberOfSymbols;
/// An array of ZXIFormat
@property(nonatomic, strong) NSArray<NSNumber*> *formats;

- (instancetype)initWithTryHarder:(BOOL)tryHarder
                        tryRotate:(BOOL)tryRotate
                     tryDownscale:(BOOL)tryDownscale
               maxNumberOfSymbols:(NSInteger)maxNumberOfSymbols
                        tryInvert:(BOOL)tryInvert
            tryCode39ExtendedMode:(BOOL)tryCode39ExtendedMode
           validateCode39CheckSum:(BOOL)validateCode39CheckSum
              validateITFCheckSum:(BOOL)validateITFCheckSum
                  downscaleFactor:(uint8_t)downscaleFactor
               downscaleThreshold:(uint16_t)downscaleThreshold
                          formats:(NSArray<NSNumber*>*)formats;
@end

NS_ASSUME_NONNULL_END
