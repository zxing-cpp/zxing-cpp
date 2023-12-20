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
                         isPure:(BOOL)isPure
                      binarizer:(ZXIBinarizer)binarizer
                downscaleFactor:(NSInteger)downscaleFactor
             downscaleThreshold:(NSInteger)downscaleThreshold
                   minLineCount:(NSInteger)minLineCount
             maxNumberOfSymbols:(NSInteger)maxNumberOfSymbols
          tryCode39ExtendedMode:(BOOL)tryCode39ExtendedMode
         validateCode39CheckSum:(BOOL)validateCode39CheckSum
            validateITFCheckSum:(BOOL)validateITFCheckSum
          returnCodabarStartEnd:(BOOL)returnCodabarStartEnd
                   returnErrors:(BOOL)returnErrors
                 eanAddOnSymbol:(ZXIEanAddOnSymbol)eanAddOnSymbol
                       textMode:(ZXITextMode)textMode {
    self = [super init];
    self.cppOpts = ZXing::ReaderOptions();
    self.formats = formats;
    self.tryHarder = tryHarder;
    self.tryRotate = tryRotate;
    self.tryInvert = tryInvert;
    self.tryDownscale = tryDownscale;
    self.isPure = isPure;
    self.binarizer = binarizer;
    self.downscaleFactor = downscaleFactor;
    self.downscaleThreshold = downscaleThreshold;
    self.minLineCount = minLineCount;
    self.maxNumberOfSymbols = maxNumberOfSymbols;
    self.tryCode39ExtendedMode = tryCode39ExtendedMode;
    self.validateCode39CheckSum = validateCode39CheckSum;
    self.validateITFCheckSum = validateITFCheckSum;
    self.returnCodabarStartEnd = returnCodabarStartEnd;
    self.returnErrors = returnErrors;
    self.eanAddOnSymbol = eanAddOnSymbol;
    self.textMode = textMode;
    return self;
}

-(BOOL)tryHarder {
    return self.cppOpts.tryHarder();
}

-(void)setTryHarder:(BOOL)tryHarder {
    self.cppOpts.setTryHarder(tryHarder);
}

-(BOOL)tryRotate {
    return self.cppOpts.tryRotate();
}

-(void)setTryRotate:(BOOL)tryRotate {
    self.cppOpts.setTryRotate(tryRotate);
}

-(BOOL)tryInvert {
    return self.cppOpts.tryInvert();
}

-(void)setTryInvert:(BOOL)tryInvert {
    self.cppOpts.setTryInvert(tryInvert);
}

-(BOOL)tryDownscale {
    return self.cppOpts.tryDownscale();
}

-(void)setTryDownscale:(BOOL)tryDownscale {
    self.cppOpts.setTryDownscale(tryDownscale);
}

-(BOOL)isPure {
    return self.cppOpts.isPure();
}

-(void)setIsPure:(BOOL)isPure {
    self.cppOpts.setIsPure(isPure);
}

-(ZXIBinarizer)binarizer {
    switch (self.cppOpts.binarizer()) {
        default:
        case ZXing::Binarizer::LocalAverage:
            return ZXIBinarizer::ZXIBinarizerLocalAverage;
        case ZXing::Binarizer::GlobalHistogram:
            return ZXIBinarizer::ZXIBinarizerGlobalHistogram;
        case ZXing::Binarizer::FixedThreshold:
            return ZXIBinarizer::ZXIBinarizerFixedThreshold;
        case ZXing::Binarizer::BoolCast:
            return ZXIBinarizer::ZXIBinarizerBoolCast;
    }
}

ZXing::Binarizer toNativeBinarizer(ZXIBinarizer binarizer) {
    switch (binarizer) {
        default:
        case ZXIBinarizerLocalAverage:
            return ZXing::Binarizer::LocalAverage;
        case ZXIBinarizerGlobalHistogram:
            return ZXing::Binarizer::GlobalHistogram;
        case ZXIBinarizerFixedThreshold:
            return ZXing::Binarizer::FixedThreshold;
        case ZXIBinarizerBoolCast:
            return ZXing::Binarizer::BoolCast;
    }
}

-(void)setBinarizer:(ZXIBinarizer)binarizer {
    self.cppOpts.setBinarizer(toNativeBinarizer(binarizer));
}

-(NSInteger)downscaleFactor {
    return self.cppOpts.downscaleFactor();
}

-(void)setDownscaleFactor:(NSInteger)downscaleFactor {
    self.cppOpts.setDownscaleFactor(downscaleFactor);
}

-(NSInteger)downscaleThreshold {
    return self.cppOpts.downscaleThreshold();
}

-(void)setDownscaleThreshold:(NSInteger)downscaleThreshold {
    self.cppOpts.setDownscaleThreshold(downscaleThreshold);
}

-(NSInteger)minLineCount {
    return self.cppOpts.minLineCount();
}

-(void)setMinLineCount:(NSInteger)minLineCount {
    self.cppOpts.setMinLineCount(minLineCount);
}

- (NSInteger)maxNumberOfSymbols {
    return self.cppOpts.maxNumberOfSymbols();
}

-(void)setMaxNumberOfSymbols:(NSInteger)maxNumberOfSymbols {
    self.cppOpts.setMaxNumberOfSymbols(maxNumberOfSymbols);
}

-(BOOL)tryCode39ExtendedMode {
    return self.cppOpts.tryCode39ExtendedMode();
}

-(void)setTryCode39ExtendedMode:(BOOL)tryCode39ExtendedMode {
    self.cppOpts.setTryCode39ExtendedMode(tryCode39ExtendedMode);
}

-(BOOL)validateCode39CheckSum {
    return self.cppOpts.validateCode39CheckSum();
}

-(void)setValidateCode39CheckSum:(BOOL)validateCode39CheckSum {
    self.cppOpts.setValidateCode39CheckSum(validateCode39CheckSum);
}

-(BOOL)validateITFCheckSum {
    return self.cppOpts.validateITFCheckSum();
}

-(void)setValidateITFCheckSum:(BOOL)validateITFCheckSum {
    self.cppOpts.setValidateITFCheckSum(validateITFCheckSum);
}

-(BOOL)returnCodabarStartEnd {
    return self.cppOpts.returnCodabarStartEnd();
}

-(void)setReturnCodabarStartEnd:(BOOL)returnCodabarStartEnd {
    self.cppOpts.setReturnCodabarStartEnd(returnCodabarStartEnd);
}

-(BOOL)returnErrors {
    return self.cppOpts.returnErrors();
}

-(void)setReturnErrors:(BOOL)returnErrors {
    self.cppOpts.setReturnErrors(returnErrors);
}

-(ZXIEanAddOnSymbol)eanAddOnSymbol {
    switch (self.cppOpts.eanAddOnSymbol()) {
        default:
        case ZXing::EanAddOnSymbol::Ignore:
            return ZXIEanAddOnSymbol::ZXIEanAddOnSymbolIgnore;
        case ZXing::EanAddOnSymbol::Read:
            return ZXIEanAddOnSymbol::ZXIEanAddOnSymbolRead;
        case ZXing::EanAddOnSymbol::Require:
            return ZXIEanAddOnSymbol::ZXIEanAddOnSymbolRequire;
    }
}

ZXing::EanAddOnSymbol toNativeEanAddOnSymbol(ZXIEanAddOnSymbol eanAddOnSymbol) {
    switch (eanAddOnSymbol) {
        default:
        case ZXIEanAddOnSymbolIgnore:
            return ZXing::EanAddOnSymbol::Ignore;
        case ZXIEanAddOnSymbolRead:
            return ZXing::EanAddOnSymbol::Read;
        case ZXIEanAddOnSymbolRequire:
            return ZXing::EanAddOnSymbol::Require;
    }
}

-(void)setEanAddOnSymbol:(ZXIEanAddOnSymbol)eanAddOnSymbol {
    self.cppOpts.setEanAddOnSymbol(toNativeEanAddOnSymbol(eanAddOnSymbol));
}

-(ZXITextMode)textMode {
    switch (self.cppOpts.textMode()) {
        default:
        case ZXing::TextMode::Plain:
            return ZXITextMode::ZXITextModePlain;
        case ZXing::TextMode::ECI:
            return ZXITextMode::ZXITextModeECI;
        case ZXing::TextMode::HRI:
            return ZXITextMode::ZXITextModeHRI;
        case ZXing::TextMode::Hex:
            return ZXITextMode::ZXITextModeHex;
        case ZXing::TextMode::Escaped:
            return ZXITextMode::ZXITextModeEscaped;
    }
}

ZXing::TextMode toNativeTextMode(ZXITextMode mode) {
    switch (mode) {
        default:
        case ZXITextModePlain:
            return ZXing::TextMode::Plain;
        case ZXITextModeECI:
            return ZXing::TextMode::ECI;
        case ZXITextModeHRI:
            return ZXing::TextMode::HRI;
        case ZXITextModeHex:
            return ZXing::TextMode::Hex;
        case ZXITextModeEscaped:
            return ZXing::TextMode::Escaped;
    }
}

-(void)setTextMode:(ZXITextMode)textMode {
    self.cppOpts.setTextMode(toNativeTextMode(textMode));
}

@end
