// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIBarcodeReader.h"
#import "ZXIErrors.h"
#import "ZXing/ReadBarcode.h"
#import "ZXing/ImageView.h"
#import "ZXing/Result.h"
#import "ZXIFormatHelper.h"

using namespace ZXing;

@interface ZXIBarcodeReader()
@property (nonatomic, strong) CIContext* ciContext;
@end

@implementation ZXIBarcodeReader

- (instancetype)init {
    return [self initWithHints: [[ZXIDecodeHints alloc]initWithTryHarder:NO tryRotate:NO formats:@[]]];
}

- (instancetype)initWithHints:(ZXIDecodeHints*)hints{
    self = [super init];
    self.ciContext = [CIContext new];
    self.hints = hints;
    return self;
}

- (nullable ZXIResult *)readCIImage:(nonnull CIImage *)image error:(NSError **)error {
    CGImageRef cgImage = [self.ciContext createCGImage:image fromRect:image.extent];
    ZXIResult *result = [self readCGImage:cgImage error:error];
    CGImageRelease(cgImage);
    return result;
}

- (nullable ZXIResult *)readCGImage: (nonnull CGImageRef)image error:(NSError **)error {
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
    CGFloat cols = CGImageGetWidth(image);
    CGFloat rows = CGImageGetHeight(image);
    NSMutableData *data = [NSMutableData dataWithLength: cols * rows];

    CGContextRef contextRef = CGBitmapContextCreate(
                                                    data.mutableBytes,// Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cols,              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags

    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image);
    CGContextRelease(contextRef);

    ImageView imageView = ImageView(
              static_cast<const uint8_t *>(data.bytes),
              static_cast<int>(cols),
              static_cast<int>(rows),
              ImageFormat::Lum);

    Result result = ReadBarcode(imageView, [ZXIBarcodeReader DecodeHintsFromZXIOptions:self.hints]);
    if(result.status() == DecodeStatus::NoError) {
        const std::wstring &resultText = result.text();
        NSString *text = [[NSString alloc] initWithBytes:resultText.data()
                                                  length:resultText.size() * sizeof(wchar_t)
                                                encoding:NSUTF32LittleEndianStringEncoding];

        std::string s(resultText.begin(), resultText.end());
        NSData *rawBytes = [[NSData alloc] initWithBytes:s.c_str() length:s.size()];
        return [[ZXIResult alloc] init:text
                                format:ZXIFormatFromBarcodeFormat(result.format())
                              rawBytes:rawBytes];
    } else {
        if(error != nil) {
            ZXIReaderError errorCode;
            switch (result.status()) {
                case ZXing::DecodeStatus::NoError:
                    // Can not happen
                    break;
                case ZXing::DecodeStatus::NotFound:
                    errorCode = ZXIReaderError::ZXINotFoundError;
                    break;
                case ZXing::DecodeStatus::FormatError:
                    errorCode = ZXIReaderError::ZXIFormatError;
                    break;
                case ZXing::DecodeStatus::ChecksumError:
                    errorCode = ZXIReaderError::ZXIChecksumError;
                    break;
            }
            *error = [[NSError alloc] initWithDomain: ZXIErrorDomain code: errorCode userInfo:nil];
        }
        return nil;
    }
}

+ (DecodeHints)DecodeHintsFromZXIOptions:(ZXIDecodeHints*)hints {
    DecodeHints resultingHints = DecodeHints();
    resultingHints.setTryRotate(hints.tryRotate);
    resultingHints.setTryHarder(hints.tryHarder);
    BarcodeFormats formats;
    for(NSNumber* flag in hints.formats) {
        formats.setFlag(BarcodeFormatFromZXIFormat((ZXIFormat)flag.integerValue));
    }
    resultingHints.setFormats(formats);
    return resultingHints;
}

@end
