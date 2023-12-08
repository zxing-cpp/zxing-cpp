// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface ZXIReaderOptions : NSObject
/// An array of ZXIFormat
@property(nonatomic, strong) NSArray<NSNumber*> *formats;
@property(nonatomic) BOOL tryHarder;
@property(nonatomic) BOOL tryRotate;
@property(nonatomic) BOOL tryInvert;
@property(nonatomic) BOOL tryDownscale;
@property(nonatomic) BOOL tryCode39ExtendedMode;
@property(nonatomic) BOOL validateCode39CheckSum;
@property(nonatomic) BOOL validateITFCheckSum;
@property(nonatomic) uint8_t downscaleFactor;
@property(nonatomic) uint16_t downscaleThreshold;
@property(nonatomic) NSInteger maxNumberOfSymbols;

- (instancetype)initWithFormats:(NSArray<NSNumber*>*)formats
					    tryHarder:(BOOL)tryHarder
                        tryRotate:(BOOL)tryRotate
                        tryInvert:(BOOL)tryInvert
                     tryDownscale:(BOOL)tryDownscale
            tryCode39ExtendedMode:(BOOL)tryCode39ExtendedMode
           validateCode39CheckSum:(BOOL)validateCode39CheckSum
              validateITFCheckSum:(BOOL)validateITFCheckSum
                  downscaleFactor:(uint8_t)downscaleFactor
               downscaleThreshold:(uint16_t)downscaleThreshold
               maxNumberOfSymbols:(NSInteger)maxNumberOfSymbols;
@end

NS_ASSUME_NONNULL_END
