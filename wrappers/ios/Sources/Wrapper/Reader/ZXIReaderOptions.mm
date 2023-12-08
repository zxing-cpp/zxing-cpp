// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIReaderOptions.h"
#import "ReaderOptions.h"

@interface ZXIReaderOptions()
@property(nonatomic) ZXing::ReaderOptions cppOpts;
@end

@implementation ZXIReaderOptions

-(instancetype)init {
    self = [super init];
    self.cppOpts = ZXing::ReaderOptions();
    return self;
}

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
               maxNumberOfSymbols:(NSInteger)maxNumberOfSymbols {
    self = [super init];
    self.cppOpts = ZXing::ReaderOptions();
    self.tryHarder = tryHarder;
    self.tryRotate = tryRotate;
    self.tryInvert = tryInvert;
    self.tryDownscale = tryDownscale;
    self.tryCode39ExtendedMode = tryCode39ExtendedMode;
    self.validateCode39CheckSum = validateCode39CheckSum;
    self.validateITFCheckSum = validateITFCheckSum;
    self.downscaleFactor = downscaleFactor;
    self.downscaleThreshold = downscaleThreshold;
    self.maxNumberOfSymbols = maxNumberOfSymbols;
    self.formats = formats;
    return self;
}

-(void)setTryHarder:(BOOL)tryHarder {
    self.cppOpts.setTryHarder(tryHarder);
}

-(void)setTryRotate:(BOOL)tryRotate {
    self.cppOpts.setTryRotate(tryRotate);
}

-(void)setTryInvert:(BOOL)tryInvert {
    self.cppOpts.setTryInvert(tryInvert);
}

-(void)setTryDownscale:(BOOL)tryDownscale {
    self.cppOpts.setTryDownscale(tryDownscale);
}

-(void)setTryCode39ExtendedMode:(BOOL)tryCode39ExtendedMode {
    self.cppOpts.setTryCode39ExtendedMode(tryCode39ExtendedMode);
}

-(void)setValidateCode39CheckSum:(BOOL)validateCode39CheckSum {
    self.cppOpts.setValidateCode39CheckSum(validateCode39CheckSum);
}

-(void)setValidateITFCheckSum:(BOOL)validateITFCheckSum {
    self.cppOpts.setValidateITFCheckSum(validateITFCheckSum);
}

-(void)setDownscaleFactor:(uint8_t)downscaleFactor {
    self.cppOpts.setDownscaleFactor(downscaleFactor);
}

-(void)setDownscaleThreshold:(uint16_t)downscaleThreshold {
    self.cppOpts.setDownscaleThreshold(downscaleThreshold);
}

-(void)setMaxNumberOfSymbols:(NSInteger)maxNumberOfSymbols {
    self.cppOpts.setMaxNumberOfSymbols(maxNumberOfSymbols);
}

-(BOOL)tryHarder {
    return self.cppOpts.tryHarder();
}

-(BOOL)tryRotate {
    return self.cppOpts.tryRotate();
}

-(BOOL)tryInvert {
    return self.cppOpts.tryInvert();
}

-(BOOL)tryDownscale {
    return self.cppOpts.tryDownscale();
}

-(BOOL)tryCode39ExtendedMode {
    return self.cppOpts.tryCode39ExtendedMode();
}

-(BOOL)validateCode39CheckSum {
    return self.cppOpts.validateCode39CheckSum();
}

-(BOOL)validateITFCheckSum {
    return self.cppOpts.validateITFCheckSum();
}

-(uint8_t)downscaleFactor {
    return self.cppOpts.downscaleFactor();
}

-(uint16_t)downscaleThreshold {
    return self.cppOpts.downscaleThreshold();
}

- (NSInteger)maxNumberOfSymbols {
    return self.cppOpts.maxNumberOfSymbols();
}

@end
