// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <CoreGraphics/CoreGraphics.h>
#import "ZXIBarcodeWriter.h"
#import "ZXIEncodeHints.h"
#import "MultiFormatWriter.h"
#import "BitMatrix.h"
#import "BitMatrixIO.h"
#import "ZXIFormatHelper.h"
#import "ZXIErrors.h"
#import <iostream>

using namespace ZXing;

std::wstring NSStringToStringW(NSString* str) {
    NSData* asData = [str dataUsingEncoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)];
    return std::wstring((wchar_t*)[asData bytes], [asData length] /
                        sizeof(wchar_t));
}

std::wstring NSDataToStringW(NSData *data) {
    std::wstring s;
    const unsigned char *bytes = (const unsigned char *) [data bytes];
    size_t len = [data length];
    for (int i = 0; i < len; ++i) {
        s.push_back(bytes[i]);
    }
    return s;
}

@implementation ZXIBarcodeWriter

-(CGImageRef)writeData:(NSData *)data
                 hints:(ZXIEncodeHints *)hints
                 error:(NSError *__autoreleasing  _Nullable *)error {
    return [self encode: NSDataToStringW(data)
               encoding: CharacterSet::BINARY
                 format: hints.format
                  width: hints.width
                 height: hints.height
                 margin: hints.margin
                ecLevel: hints.ecLevel
                  error: error];
}

-(CGImageRef)writeString:(NSString *)contents
                   hints:(ZXIEncodeHints *)hints
                   error:(NSError *__autoreleasing  _Nullable *)error {
    return [self encode: NSStringToStringW(contents)
               encoding: CharacterSet::UTF8
                 format: hints.format
                  width: hints.width
                 height: hints.height
                 margin: hints.margin
                ecLevel: hints.ecLevel
                  error: error];
}

-(CGImageRef)encode:(std::wstring)content
           encoding:(CharacterSet)encoding
             format:(ZXIFormat)format
              width:(int)width
             height:(int)height
             margin:(int)margin
            ecLevel:(int)ecLevel
              error:(NSError *__autoreleasing  _Nullable *)error {
    MultiFormatWriter writer { BarcodeFormatFromZXIFormat(format) };
    writer.setEncoding(encoding);
    writer.setMargin(margin);
    writer.setEccLevel(ecLevel);
    // Catch exception for invalid formats
    try {
        BitMatrix bitMatrix = writer.encode(content, width, height);
        return [self inflate:&bitMatrix];
    } catch(std::exception &e) {
        if (error != nil) {
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: [[NSString alloc] initWithUTF8String:e.what()]
            };
            *error = [[NSError alloc] initWithDomain:ZXIErrorDomain code:ZXIWriterError userInfo:userInfo];
        }
        return nil;
    }
}

-(CGImageRef)inflate:(BitMatrix *)bitMatrix {
    int realWidth = bitMatrix->width();
    int realHeight = bitMatrix->height();

#ifdef DEBUG
    std::cout << ToString(*bitMatrix, 'X', ' ', false, false);
#endif

    NSMutableData *resultAsNSData = [[NSMutableData alloc] initWithLength:realWidth * realHeight];
    size_t index = 0;
    uint8_t *bytes = (uint8_t*)resultAsNSData.mutableBytes;
    for (int y = 0; y < realHeight; ++y) {
        for (int x = 0; x < realWidth; ++x) {
            bytes[index] = bitMatrix->get(x, y) ? 0 : 255;
            ++index;
        }
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);

    return CGImageCreate(realWidth,
                         realHeight,
                         8,
                         8,
                         realWidth,
                         colorSpace,
                         kCGBitmapByteOrderDefault,
                         CGDataProviderCreateWithCFData((CFDataRef)resultAsNSData),
                         NULL,
                         YES,
                         kCGRenderingIntentDefault);
}

@end
