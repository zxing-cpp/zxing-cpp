//
//  BarcodeReader.h
//  
//
//  Created by Christian Braun on 22.02.22.
//

#import <CoreGraphics/CoreGraphics.h>
#import <CoreImage/CoreImage.h>
#import "ZXIResult.h"
#import "ZXIDecodeHints.h"

NS_ASSUME_NONNULL_BEGIN

@interface ZXIBarcodeReader : NSObject
@property(nonatomic, strong) ZXIDecodeHints *hints;

- (instancetype)initWithHints:(ZXIDecodeHints*)options;
- (nullable ZXIResult *)readCIImage:(nonnull CIImage *)image error:(NSError **)error;
- (nullable ZXIResult *)readCGImage:(nonnull CGImageRef)image error:(NSError **)error;
@end

NS_ASSUME_NONNULL_END
