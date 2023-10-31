// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIBarcodeReader.h"
#import "ReadBarcode.h"
#import "ImageView.h"
#import "Result.h"
#import "GTIN.h"
#import "ZXIFormatHelper.h"
#import "ZXIPosition+Helper.h"

using namespace ZXing;

NSString *stringToNSString(const std::string &text) {
    return [[NSString alloc]initWithBytes:text.data() length:text.size() encoding:NSUTF8StringEncoding];
}

ZXIGTIN *getGTIN(const Result &result) {
    auto country = GTIN::LookupCountryIdentifier(result.text(TextMode::Plain), result.format());
    auto addOn = GTIN::EanAddOn(result);
    return country.empty()
        ? nullptr
        : [[ZXIGTIN alloc]initWithCountry:stringToNSString(country)
                                    addOn:stringToNSString(addOn)
                                    price:stringToNSString(GTIN::Price(addOn))
                              issueNumber:stringToNSString(GTIN::IssueNr(addOn))];
}

@interface ZXIBarcodeReader()
@property (nonatomic, strong) CIContext* ciContext;
@end

@implementation ZXIBarcodeReader

- (instancetype)init {
    return [self initWithHints: [[ZXIDecodeHints alloc] init]];
}

- (instancetype)initWithHints:(ZXIDecodeHints*)hints{
    self = [super init];
    self.ciContext = [[CIContext alloc] initWithOptions:@{kCIContextWorkingColorSpace: [NSNull new]}];
    self.hints = hints;
    return self;
}

- (NSArray<ZXIResult *> *)readCVPixelBuffer:(nonnull CVPixelBufferRef)pixelBuffer {
    OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);

    // We tried to work with all luminance based formats listed in kCVPixelFormatType
    // but only the following ones seem to be supported on iOS.
    switch (pixelFormat) {
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
            NSInteger cols = CVPixelBufferGetWidth(pixelBuffer);
            NSInteger rows = CVPixelBufferGetHeight(pixelBuffer);
            NSInteger bytesPerRow = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            const uint8_t * bytes = static_cast<const uint8_t *>(CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0));
            ImageView imageView = ImageView(
                                            static_cast<const uint8_t *>(bytes),
                                            static_cast<int>(cols),
                                            static_cast<int>(rows),
                                            ImageFormat::Lum,
                                            static_cast<int>(bytesPerRow),
                                            0);
            NSArray* results = [self readImageView:imageView];
            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            return results;
    }

    // If given pixel format is not a supported type with a luminance channel we just use the
    // default method
    return [self readCIImage:[[CIImage alloc] initWithCVImageBuffer:pixelBuffer]];
}

- (NSArray<ZXIResult *> *)readCIImage:(nonnull CIImage *)image {
    CGImageRef cgImage = [self.ciContext createCGImage:image fromRect:image.extent];
    auto results = [self readCGImage:cgImage];
    CGImageRelease(cgImage);
    return results;
}

- (NSArray<ZXIResult *> *)readCGImage: (nonnull CGImageRef)image {
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
    return [self readImageView:imageView];
}

+ (DecodeHints)DecodeHintsFromZXIOptions:(ZXIDecodeHints*)hints {
    BarcodeFormats formats;
    for(NSNumber* flag in hints.formats) {
        formats.setFlag(BarcodeFormatFromZXIFormat((ZXIFormat)flag.integerValue));
    }
    DecodeHints resultingHints = DecodeHints()
        .setTryRotate(hints.tryRotate)
        .setTryHarder(hints.tryHarder)
        .setTryInvert(hints.tryInvert)
        .setTryDownscale(hints.tryDownscale)
        .setTryCode39ExtendedMode(hints.tryCode39ExtendedMode)
        .setValidateCode39CheckSum(hints.validateCode39CheckSum)
        .setValidateITFCheckSum(hints.validateITFCheckSum)

        .setFormats(formats)
        .setMaxNumberOfSymbols(hints.maxNumberOfSymbols);
    return resultingHints;
}

- (NSArray<ZXIResult*> *)readImageView: (ImageView)imageView {
    Results results = ReadBarcodes(imageView, [ZXIBarcodeReader DecodeHintsFromZXIOptions:self.hints]);

    NSMutableArray* zxiResults = [NSMutableArray array];
    for (auto result: results) {
        [zxiResults addObject:
         [[ZXIResult alloc] init:stringToNSString(result.text())
                          format:ZXIFormatFromBarcodeFormat(result.format())
                           bytes:[[NSData alloc] initWithBytes:result.bytes().data() length:result.bytes().size()]
                        position:[[ZXIPosition alloc]initWithPosition: result.position()]
                     orientation:result.orientation()
                         ecLevel:stringToNSString(result.ecLevel())
             symbologyIdentifier:stringToNSString(result.symbologyIdentifier())
                    sequenceSize:result.sequenceSize()
                   sequenceIndex:result.sequenceIndex()
                      sequenceId:stringToNSString(result.sequenceId())
                      readerInit:result.readerInit()
                       lineCount:result.lineCount()
                            gtin:getGTIN(result)]
         ];
    }
    return zxiResults;
}

@end
