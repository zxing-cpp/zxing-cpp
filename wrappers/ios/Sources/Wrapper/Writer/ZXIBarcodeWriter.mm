// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <CoreGraphics/CoreGraphics.h>
#import "ZXIBarcodeWriter.h"
#import "ZXing/MultiFormatWriter.h"
#import "ZXing/BitMatrix.h"
#import "ZXIFormatHelper.h"
#import "ZXIErrors.h"
#import <iostream>

using namespace ZXing;

std::wstring NSStringToStringW(NSString* str) {
    NSData* asData = [str dataUsingEncoding:CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE)];
    return std::wstring((wchar_t*)[asData bytes], [asData length] /
                        sizeof(wchar_t));
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

-(CGImageRef)write:(NSString *)contents
             width:(int)width
            height:(int)height
            format:(ZXIFormat)format
             error:(NSError *__autoreleasing  _Nullable *)error {
    MultiFormatWriter writer { BarcodeFormatFromZXIFormat(format) };
    // Catch exception for invalid formats
    try {
        BitMatrix result = writer.encode(NSStringToStringW(contents), width, height);
        int realWidth = result.width();
        int realHeight = result.height();

#ifdef DEBUG
//        std::cout << ToString(result, 'X', ' ', false, false);
#endif

        NSMutableData *resultAsNSData = [[NSMutableData alloc] initWithLength:realWidth * realHeight];
        size_t index = 0;
        uint8_t *bytes = (uint8_t*)resultAsNSData.mutableBytes;
        for (int y = 0; y < realHeight; ++y) {
            for (int x = 0; x < realWidth; ++x) {
                bytes[index] = result.get(x, y) ? 0 : 255;
                ++index;
            }
        }

        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);

        CGImageRef cgimage = CGImageCreate(realWidth,
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
        return cgimage;
    } catch(std::exception &e) {
        if(error != nil) {
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: [[NSString alloc] initWithUTF8String:e.what()]
            };
            *error = [[NSError alloc] initWithDomain:ZXIErrorDomain code:ZXIWriterError userInfo:userInfo];
        }
        return nil;
    }
}

@end
