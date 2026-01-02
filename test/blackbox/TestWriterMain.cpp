/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "CreateBarcode.h"
#include "WriteBarcode.h"

#include <vector>

using namespace ZXing;
using namespace std::literals;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void savePng(ImageView iv, BarcodeFormat format)
{
	stbi_write_png((ToString(format) + ".png"s).c_str(), iv.width(), iv.height(), iv.pixStride(), iv.data(), iv.rowStride());
}

int main()
{
	std::string text = "http://www.google.com/";
	for (auto format : {
#ifdef ZXING_WITH_AZTEC
		BarcodeFormat::Aztec,
#endif
#ifdef ZXING_WITH_DATAMATRIX
		BarcodeFormat::DataMatrix,
#endif
#ifdef ZXING_WITH_PDF417
		BarcodeFormat::PDF417,
#endif
#ifdef ZXING_WITH_QRCODE
		BarcodeFormat::QRCode,
#endif
	})
	{
		savePng(CreateBarcodeFromText(text, format).symbol(), format);
	}

	text = "012345678901234567890123456789";
	using FormatSpecs = std::vector<std::pair<BarcodeFormat, size_t>>;
	for (const auto& [format, length] : FormatSpecs({
#ifdef ZXING_WITH_1D
//		{BarcodeFormat::Codabar, 0},
		{BarcodeFormat::Code39, 0},
		{BarcodeFormat::Code93, 0},
		{BarcodeFormat::Code128, 0},
		{BarcodeFormat::EAN8, 7},
		{BarcodeFormat::EAN13, 12},
		{BarcodeFormat::ITF, 0},
		{BarcodeFormat::UPCA, 11},
		{BarcodeFormat::UPCE, 7}
#endif
	}))
	{
		auto input = length > 0 ? text.substr(0, length) : text;
		savePng(CreateBarcodeFromText(input, format).symbol(), format);
	}
}
