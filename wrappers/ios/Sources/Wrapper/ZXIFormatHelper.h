// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "BarcodeFormat.h"
#import "ZXIFormat.h"

NS_ASSUME_NONNULL_BEGIN

ZXing::BarcodeFormat BarcodeFormatFromZXIFormat(ZXIFormat format);
ZXIFormat ZXIFormatFromBarcodeFormat(ZXing::BarcodeFormat format);
NS_ASSUME_NONNULL_END
