//
//  ZXIFormatHelper.h
//  
//
//  Created by Christian Braun on 25.03.22.
//

#import <Foundation/Foundation.h>
#import "ZXing/BarcodeFormat.h"
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

ZXing::BarcodeFormat BarcodeFormatFromZXIFormat(ZXIFormat format);
ZXIFormat ZXIFormatFromBarcodeFormat(ZXing::BarcodeFormat format);
NS_ASSUME_NONNULL_END
