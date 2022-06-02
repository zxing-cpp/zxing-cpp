// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

#define ZXIErrorDomain @"ZXIErrorDomain"

typedef NS_ENUM(NSInteger, ZXIReaderError) {
    ZXINotFoundError = 1000,
    ZXIChecksumError,
    ZXIFormatError,
    ZXIWriterError,
};

NS_ASSUME_NONNULL_END
