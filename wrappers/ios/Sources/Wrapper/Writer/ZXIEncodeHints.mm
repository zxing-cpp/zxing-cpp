// Copyright 2023 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIEncodeHints.h"

@implementation ZXIEncodeHints

- (instancetype)initWithFormat:(ZXIFormat)format {
    self = [super init];
    self.format = format;
    self.width = 0;
    self.height = 0;
    self.ecLevel = -1;
    self.margin = -1;
    return self;
}

- (instancetype)initWithFormat:(ZXIFormat)format
                         width:(int)width
                        height:(int)height
                       ecLevel:(int)ecLevel
                        margin:(int)margin {
    self = [super init];
    self.format = format;
    self.width = width;
    self.height = height;
    self.ecLevel = ecLevel;
    self.margin = margin;
    return self;
}

-(void)setFormat:(ZXIFormat)format {
    self.format = format;
}

-(void)setWidth:(int)width {
    self.width = width;
}

-(void)setHeight:(int)height {
    self.height = height;
}

-(void)setEcLevel:(int)ecLevel {
    self.ecLevel = ecLevel;
}

-(void)setMargin:(int)margin {
    self.margin = margin;
}

@end
