// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"
#include "ImageView.h"
#include "wxcode/WXGrid.h"

#include <vector>

namespace ZXing::WXCode::Test {

constexpr int IMAGE_SIZE = 305;

Grid EncodeData(const ByteArray& data, unsigned version, unsigned ec);
Grid Encode(const ByteArray& payload, unsigned version = 0, unsigned ec = 0);
std::vector<uint8_t> Render(const Grid& grid);

} // namespace ZXing::WXCode::Test
