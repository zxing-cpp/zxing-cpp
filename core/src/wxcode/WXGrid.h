// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>
#include <cstdint>

namespace ZXing::WXCode {

inline constexpr int ANGULAR_ROWS = 72;
inline constexpr int RADIAL_CELLS = 16;

// Angular rows increase clockwise from the left; radius zero is innermost.
using Grid = std::array<std::array<uint8_t, RADIAL_CELLS>, ANGULAR_ROWS>;

} // namespace ZXing::WXCode
