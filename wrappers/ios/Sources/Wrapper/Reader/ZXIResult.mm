//
//  ZXIResult.m
//  
//
//  Created by Christian Braun on 25.02.22.
//

#import "ZXIResult.h"

@implementation ZXIResult
- (instancetype)init:(NSString *)text
              format:(ZXIFormat)format
            rawBytes:(NSData *)rawBytes {
    self = [super init];
    self.text = text;
    self.format = format;
    self.rawBytes = rawBytes;
    return self;
}
@end
