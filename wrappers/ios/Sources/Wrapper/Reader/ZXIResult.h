// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"
#import "ZXIPosition.h"
#import "ZXIGTIN.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIResult : NSObject
@property(nonatomic, strong) NSString *text;
@property(nonatomic, strong) NSData *bytes;
@property(nonatomic, strong) ZXIPosition *position;
@property(nonatomic) ZXIFormat format;
@property(nonatomic) NSInteger orientation;
@property(nonatomic, strong) NSString *ecLevel;
@property(nonatomic, strong) NSString *symbologyIdentifier;
@property(nonatomic) NSInteger sequenceSize;
@property(nonatomic) NSInteger sequenceIndex;
@property(nonatomic, strong) NSString *sequenceId;
@property(nonatomic) BOOL readerInit;
@property(nonatomic) NSInteger lineCount;
@property(nonatomic, strong) ZXIGTIN *gtin;

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
                gtin:(ZXIGTIN *)gtin;
@end

NS_ASSUME_NONNULL_END
