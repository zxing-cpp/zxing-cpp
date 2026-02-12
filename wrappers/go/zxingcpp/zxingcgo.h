// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if defined(__has_include)
#if __has_include(<ZXing/ZXingC.h>)
#include <ZXing/ZXingC.h>
#elif __has_include("ZXingC.h")
#include "ZXingC.h"
#else
#error "ZXingC.h not found. Build zxing-cpp with -DZXING_C_API=ON or install zxing-cpp development headers."
#endif
#else
#include <ZXing/ZXingC.h>
#endif
