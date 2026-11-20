// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <string_view>

namespace ZXing::WXCode {

inline constexpr int MAX_IMAGE_DIMENSION = 16384;
inline constexpr int MAX_IMAGE_PIXELS = 32 * 1024 * 1024;
inline constexpr size_t MAX_PAYLOAD_BYTES = 160;
inline constexpr std::string_view ALPHANUMERIC_ALPHABET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

inline bool IsSupportedImageSize(int width, int height)
{
	return width > 0 && height > 0 && width <= MAX_IMAGE_DIMENSION && height <= MAX_IMAGE_DIMENSION
		   && width <= MAX_IMAGE_PIXELS / height;
}

} // namespace ZXing::WXCode
