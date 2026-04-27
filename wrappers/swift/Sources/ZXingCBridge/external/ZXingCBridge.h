// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#if __has_include(<ZXing/ZXingC.h>)
#include <ZXing/ZXingC.h>
#elif __has_include("ZXingC.h")
#include "ZXingC.h"
#else
#error "ZXingC.h not found. Install zxing-cpp headers or enable bundled mode (ZXING_BUNDLED=1)."
#endif
