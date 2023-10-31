// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIResult.h"

@implementation ZXIResult
- (instancetype)init:(NSString *)text
              format:(ZXIFormat)format
               bytes:(NSData *)bytes
            position:(ZXIPosition *)position
         orientation:(NSInteger)orientation
             ecLevel:(NSString *)ecLevel
 symbologyIdentifier:(NSString *)symbologyIdentifier
        sequenceSize:(NSInteger)sequenceSize
       sequenceIndex:(NSInteger)sequenceIndex
          sequenceId:(NSString *)sequenceId
          readerInit:(BOOL)readerInit
           lineCount:(NSInteger)lineCount
                gtin:(ZXIGTIN *)gtin {
    self = [super init];
    self.text = text;
    self.format = format;
    self.bytes = bytes;
    self.position = position;
    self.orientation = orientation;
    self.ecLevel = ecLevel;
    self.symbologyIdentifier = symbologyIdentifier;
    self.sequenceSize = sequenceSize;
    self.sequenceIndex = sequenceIndex;
    self.sequenceId = sequenceId;
    self.readerInit = readerInit;
    self.lineCount = lineCount;
    self.gtin = gtin;
    return self;
}
@end
