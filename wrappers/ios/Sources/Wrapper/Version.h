/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

// This header a 'manual' rendition of Version.h.in for the iOS (SwiftPM / Cocoapods) build systems that don't use cmake

#pragma once

#define ZXING_READERS
#define ZXING_WRITERS

#define ZXING_ENABLE_1D 1
#define ZXING_ENABLE_AZTEC 1
#define ZXING_ENABLE_DATAMATRIX 1
#define ZXING_ENABLE_MAXICODE 1
#define ZXING_ENABLE_PDF417 1
#define ZXING_ENABLE_QRCODE 1

// #define ZXING_EXPERIMENTAL_API
// #define ZXING_USE_ZINT

// Version numbering
#define ZXING_VERSION_MAJOR 3
#define ZXING_VERSION_MINOR 0
#define ZXING_VERSION_PATCH 0
#define ZXING_VERSION_SUFFIX ""

#define ZXING_VERSION_STR "3.0.0" ZXING_VERSION_SUFFIX
