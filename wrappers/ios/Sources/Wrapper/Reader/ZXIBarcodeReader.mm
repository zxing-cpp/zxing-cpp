// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import "ZXIBarcodeReader.h"
#import "ReadBarcode.h"
#import "ImageView.h"
#import "Barcode.h"
#import "GTIN.h"
#import "ZXIFormatHelper.h"
#import "ZXIPosition+Helper.h"
#import "ZXIErrors.h"

using namespace ZXing;

NSString *stringToNSString(const std::string &text) {
    return [[NSString alloc]initWithBytes:text.data() length:text.size() encoding:NSUTF8StringEncoding];
}

ZXIGTIN *getGTIN(const Result &result) {
    try {
        auto country = GTIN::LookupCountryIdentifier(result.text(TextMode::Plain), result.format());
        auto addOn = GTIN::EanAddOn(result);
        return country.empty()
            ? nullptr
            : [[ZXIGTIN alloc]initWithCountry:stringToNSString(country)
                                        addOn:stringToNSString(addOn)
                                        price:stringToNSString(GTIN::Price(addOn))
                                  issueNumber:stringToNSString(GTIN::IssueNr(addOn))];
    } catch (std::exception e) {
        // Because invalid GTIN data can lead to exceptions, in which case
        // we don't want to discard the whole result.
        return nullptr;
    }
}

@interface ZXIReaderOptions()
@property(nonatomic) ZXing::ReaderOptions cppOpts;
@end

@interface ZXIBarcodeReader()
@property (nonatomic, strong) CIContext* ciContext;
@end

@implementation ZXIBarcodeReader

- (instancetype)init {
    return [self initWithOptions: [[ZXIReaderOptions alloc] init]];
}

- (instancetype)initWithOptions:(ZXIReaderOptions*)options{
    self = [super init];
    self.ciContext = [[CIContext alloc] initWithOptions:@{kCIContextWorkingColorSpace: [NSNull new]}];
    self.options = options;
    return self;
}

- (NSArray<ZXIResult *> *)readCVPixelBuffer:(nonnull CVPixelBufferRef)pixelBuffer
                                      error:(NSError *__autoreleasing _Nullable *)error {
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
            NSArray* results = [self readImageView:imageView error:error];
            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            return results;
    }

    // If given pixel format is not a supported type with a luminance channel we just use the
    // default method
    return [self readCIImage:[[CIImage alloc] initWithCVImageBuffer:pixelBuffer] error:error];
}

- (NSArray<ZXIResult *> *)readCIImage:(nonnull CIImage *)image
                                error:(NSError *__autoreleasing _Nullable *)error {
    CGImageRef cgImage = [self.ciContext createCGImage:image fromRect:image.extent];
    auto results = [self readCGImage:cgImage error:error];
    CGImageRelease(cgImage);
    return results;
}

- (NSArray<ZXIResult *> *)readCGImage:(nonnull CGImageRef)image
                                error:(NSError *__autoreleasing _Nullable *)error {
    CGFloat cols = CGImageGetWidth(image);
    CGFloat rows = CGImageGetHeight(image);
    NSMutableData *data = [NSMutableData dataWithLength: cols * rows];

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
    CGContextRef contextRef = CGBitmapContextCreate(data.mutableBytes,// Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cols,              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image);
    CGContextRelease(contextRef);

    ImageView imageView = ImageView(
              static_cast<const uint8_t *>(data.bytes),
              static_cast<int>(cols),
              static_cast<int>(rows),
              ImageFormat::Lum);
    return [self readImageView:imageView error:error];
}

- (NSArray<ZXIResult*> *)readImageView:(ImageView)imageView
                                 error:(NSError *__autoreleasing _Nullable *)error {
    try {
        Barcodes results = ReadBarcodes(imageView, self.options.cppOpts);
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
    } catch(std::exception &e) {
        SetNSError(error, ZXIReaderError, e.what());
        return nil;
    } catch (...) {
        SetNSError(error, ZXIReaderError, "An unknown error occurred");
        return nil;
    }
}

@end
