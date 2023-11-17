// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#define ZXIErrorDomain @"ZXIErrorDomain"

typedef NS_ENUM(NSInteger, ZXIBarcodeReaderError) {
    ZXIReaderError,
};

typedef NS_ENUM(NSInteger, ZXIBarcodeWriterError) {
    ZXIWriterError,
};

void SetNSError(NSError *__autoreleasing _Nullable* error, NSInteger code, const char* message);

NS_ASSUME_NONNULL_END
