/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrix.h"
#include "MultiFormatWriter.h"

#include <vector>

using namespace ZXing;
using namespace std::literals;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void savePng(const BitMatrix& matrix, BarcodeFormat format)
{
	auto bitmap = ToMatrix<uint8_t>(matrix);
	stbi_write_png((ToString(format) + ".png"s).c_str(), bitmap.width(), bitmap.height(), 1, bitmap.data(), 0);
}

int main()
{
	std::string text = "http://www.google.com/";
	for (auto format : {
		BarcodeFormat::Aztec,
		BarcodeFormat::DataMatrix,
		BarcodeFormat::PDF417,
		BarcodeFormat::QRCode })
	{
		savePng(MultiFormatWriter(format).encode(text, 200, 200), format);
	}

	text = "012345678901234567890123456789";
	using FormatSpecs = std::vector<std::pair<BarcodeFormat, size_t>>;
	for (const auto& [format, length] : FormatSpecs({
		{BarcodeFormat::Codabar, 0},
		{BarcodeFormat::Code39, 0},
		{BarcodeFormat::Code93, 0},
		{BarcodeFormat::Code128, 0},
		{BarcodeFormat::EAN8, 7},
		{BarcodeFormat::EAN13, 12},
		{BarcodeFormat::ITF, 0},
		{BarcodeFormat::UPCA, 11},
		{BarcodeFormat::UPCE, 7} }))
	{
		auto input = length > 0 ? text.substr(0, length) : text;
		savePng(MultiFormatWriter(format).encode(input, 100, 100), format);
	}
}
