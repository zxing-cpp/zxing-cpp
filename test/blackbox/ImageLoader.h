/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ImageView.h"
#include "ZXFilesystem.h"

namespace ZXing::Test::ImageLoader {

void clearCache();
const ImageView& load(const fs::path& imgPath);

} // namespace ZXing::Test::ImageLoader
