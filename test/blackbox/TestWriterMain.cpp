/*
* Copyright 2016 Nu-book Inc.
* Copyright 2019 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

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
	std::wstring text = L"http://www.google.com/";
	for (auto format : {
		BarcodeFormat::Aztec,
		BarcodeFormat::DataMatrix,
		BarcodeFormat::PDF417,
		BarcodeFormat::QRCode })
	{
		savePng(MultiFormatWriter(format).encode(text, 200, 200), format);
	}

	text = L"012345678901234567890123456789";
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
