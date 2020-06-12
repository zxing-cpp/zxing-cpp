/*
* Copyright 2016 Nu-book Inc.
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

#include "ReadBarcode.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <emscripten/bind.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct ReadResult
{
	std::string format;
	std::wstring text;
	std::string error;
};

ReadResult readBarcodeFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));

		int width, height, channels;
		std::unique_ptr<stbi_uc, void (*)(void*)> buffer(
			stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bufferPtr), bufferLength, &width, &height,
								  &channels, 4),
			stbi_image_free);
		if (buffer == nullptr) {
			return { "", L"", "Error loading image" };
		}

		auto result = ReadBarcode({buffer.get(), width, height, ImageFormat::RGBX}, hints);
		if (result.isValid()) {
			return { ToString(result.format()), result.text(), "" };
		}
	}
	catch (const std::exception& e) {
		return { "", L"", e.what() };
	}
	catch (...) {
		return { "", L"", "Unknown error" };
	}
	return {};
}

ReadResult readBarcodeFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));

		auto result =
			ReadBarcode({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBX}, hints);

		if (result.isValid()) {
			return { ToString(result.format()), result.text(), "" };
		}
	}
	catch (const std::exception& e) {
		return { "", L"", e.what() };
	}
	catch (...) {
		return { "", L"", "Unknown error" };
	}
	return {};
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
	using namespace emscripten;

	value_object<ReadResult>("ReadResult")
	        .field("format", &ReadResult::format)
	        .field("text", &ReadResult::text)
	        .field("error", &ReadResult::error)
	        ;

	function("readBarcodeFromImage", &readBarcodeFromImage);
	function("readBarcodeFromPixmap", &readBarcodeFromPixmap);

	// obsoletes
	function("readBarcode", &readBarcodeFromImage);
	function("readBarcodeFromPng", &readBarcodeFromImage);
}
