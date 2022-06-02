//
//  ZXIBarcodeWriter.h
//  
//
//  Created by Hendrik von Prince on 25.02.22.
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXIFormat.h"
#import "ZXICharset.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeWriter : NSObject

-(nullable CGImageRef)write:(NSString *)contents
                      width:(int)width
                     height:(int)height
                     format:(ZXIFormat)format
                      error:(NSError **)error;

-(nullable CGImageRef)write:(NSString *)contents
                      width:(int)width
                     height:(int)height
                     format:(ZXIFormat)format
                    charset:(ZXICharset)charset
                      error:(NSError **)error;
@end

NS_ASSUME_NONNULL_END
