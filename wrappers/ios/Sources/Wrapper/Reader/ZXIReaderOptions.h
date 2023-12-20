// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, ZXIBinarizer) {
    ZXIBinarizerLocalAverage,
    ZXIBinarizerGlobalHistogram,
    ZXIBinarizerFixedThreshold,
    ZXIBinarizerBoolCast
};

typedef NS_ENUM(NSInteger, ZXIEanAddOnSymbol) {
    ZXIEanAddOnSymbolIgnore,
    ZXIEanAddOnSymbolRead,
    ZXIEanAddOnSymbolRequire
};

typedef NS_ENUM(NSInteger, ZXITextMode) {
    ZXITextModePlain,
    ZXITextModeECI,
    ZXITextModeHRI,
    ZXITextModeHex,
    ZXITextModeEscaped
};

@interface ZXIReaderOptions : NSObject
/// An array of ZXIFormat
@property(nonatomic, strong) NSArray<NSNumber*> *formats;
@property(nonatomic) BOOL tryHarder;
@property(nonatomic) BOOL tryRotate;
@property(nonatomic) BOOL tryInvert;
@property(nonatomic) BOOL tryDownscale;
@property(nonatomic) BOOL isPure;
@property(nonatomic) ZXIBinarizer binarizer;
@property(nonatomic) NSInteger downscaleFactor;
@property(nonatomic) NSInteger downscaleThreshold;
@property(nonatomic) NSInteger minLineCount;
@property(nonatomic) NSInteger maxNumberOfSymbols;
@property(nonatomic) BOOL tryCode39ExtendedMode;
@property(nonatomic) BOOL validateCode39CheckSum;
@property(nonatomic) BOOL validateITFCheckSum;
@property(nonatomic) BOOL returnCodabarStartEnd;
@property(nonatomic) BOOL returnErrors;
@property(nonatomic) ZXIEanAddOnSymbol eanAddOnSymbol;
@property(nonatomic) ZXITextMode textMode;

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
                       textMode:(ZXITextMode)textMode;
@end

NS_ASSUME_NONNULL_END
