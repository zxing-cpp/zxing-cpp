/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingCpp.h"

#include <vector>

using namespace ZXing;
using namespace std::literals;
using enum BarcodeFormat;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void savePng(ImageView iv, BarcodeFormat format)
{
	stbi_write_png((ToString(format) + ".png"s).c_str(), iv.width(), iv.height(), iv.pixStride(), iv.data(), iv.rowStride());
}

int main()
{
	std::string text = "zxing-cpp";
	for (auto format : BarcodeFormats::list(AllMatrix))
		if (format & AllCreatable && format != AztecRune)
			savePng(CreateBarcodeFromText(text, format).symbol(), format);

	text = "012345678901234567890123456789";
	using FormatSpecs = std::vector<std::pair<BarcodeFormat, size_t>>;
	for (const auto& [format, length] : FormatSpecs({
		// {Codabar, 0}, // needs to start with A, B, C or D
		{Code39, 0},
		{Code93, 0},
		{Code128, 0},
		{EAN8, 7},
		{EAN13, 12},
		{ITF, 0},
		{UPCA, 11},
		{UPCE, 7}
	}))
	{
		auto input = length > 0 ? text.substr(0, length) : text;
		if (format & AllCreatable)
			savePng(WriteBarcodeToImage(CreateBarcodeFromText(input, format)), format);
	}
}
