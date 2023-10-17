// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <CoreGraphics/CoreGraphics.h>
#import "ZXIBarcodeWriter.h"
#import "ZXIFormatHelper.h"
#import "ZXIErrors.h"
#import "ZXing/MultiFormatWriter.h"
#import "ZXing/BitMatrix.h"
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
    wchar_t buf[len];
    for (int i = 0; i < len; ++i) {
        buf[i] = bytes[i];
    }
    s.assign(buf, len);
    return s;
}

#ifdef  DEBUG
std::string ToString(const BitMatrix& matrix, char one, char zero, bool addSpace, bool printAsCString)
{
    std::string result;
    result.reserve((addSpace ? 2 : 1) * (matrix.width() * matrix.height()) + matrix.height());
    for (int y = 0; y < matrix.height(); ++y) {
        for (int x = 0; x < matrix.width(); ++x) {
            result += matrix.get(x, y) ? one : zero;
            if (addSpace)
                result += ' ';
        }
        if (printAsCString)
            result += "\\n\"";
        result += '\n';
    }
    return result;
}
#endif

@implementation ZXIBarcodeWriter

-(CGImageRef)writeData:(NSData *)data
                 width:(int)width
                height:(int)height
                format:(ZXIFormat)format
                 error:(NSError *__autoreleasing  _Nullable *)error {
    return [self encode: NSDataToStringW(data)
                  width: width
                 height: height
                 format: format
               encoding: CharacterSet::BINARY
                  error: error];
}

-(CGImageRef)write:(NSString *)contents
             width:(int)width
            height:(int)height
            format:(ZXIFormat)format
             error:(NSError *__autoreleasing  _Nullable *)error {
    return [self encode: NSStringToStringW(contents)
                  width: width
                 height: height
                 format: format
               encoding: CharacterSet::UTF8
                  error: error];
}

-(CGImageRef)encode:(std::wstring)content
              width:(int)width
             height:(int)height
             format:(ZXIFormat)format
           encoding:(CharacterSet)encoding
              error:(NSError *__autoreleasing  _Nullable *)error {
    MultiFormatWriter writer { BarcodeFormatFromZXIFormat(format) };
    writer.setEncoding(encoding);
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
    std::cout << ToString(bitMatrix, 'X', ' ', false, false);
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
