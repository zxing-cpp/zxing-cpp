// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>
#import "ZXing/CharacterSet.h"
#import "ZXICharset.h"

NS_ASSUME_NONNULL_BEGIN

ZXICharset ZXICharsetFromCharset(ZXing::CharacterSet charset);
ZXing::CharacterSet CharsetFromZXICharset(ZXICharset charset);

NS_ASSUME_NONNULL_END
