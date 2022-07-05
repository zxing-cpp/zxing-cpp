/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#define ZX_USE_UTF8 1 // see Result.h

#include "ReadBarcode.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <emscripten/bind.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct ReadResult
{
	std::string format;
	std::string text;
	std::string error;
	ZXing::Position position;
};

ReadResult readBarcodeFromImageView(ZXing::ImageView iv, bool tryHarder, const std::string& format)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setTryDownscale(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));
		hints.setMaxNumberOfSymbols(1);

		auto results = ReadBarcodes(iv, hints);
		if (!results.empty()) {
			auto& result = results.front();
			return { ToString(result.format()), result.text(), "", result.position() };
		}
	}
	catch (const std::exception& e) {
		return { "", "", e.what() };
	}
	catch (...) {
		return { "", "", "Unknown error" };
	}
	return {};
}

ReadResult readBarcodeFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
	using namespace ZXing;

	int width, height, channels;
	std::unique_ptr<stbi_uc, void (*)(void*)> buffer(
		stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bufferPtr), bufferLength, &width, &height, &channels, 4),
		stbi_image_free);
	if (buffer == nullptr) {
		return {"", "", "Error loading image"};
	}

	return readBarcodeFromImageView({buffer.get(), width, height, ImageFormat::RGBX}, tryHarder, format);
}

ReadResult readBarcodeFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format)
{
	using namespace ZXing;
	return readBarcodeFromImageView({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBX}, tryHarder, format);
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
	using namespace emscripten;

	value_object<ReadResult>("ReadResult")
			.field("format", &ReadResult::format)
			.field("text", &ReadResult::text)
			.field("error", &ReadResult::error)
			.field("position", &ReadResult::position)
			;

	value_object<ZXing::PointI>("Point")
			.field("x", &ZXing::PointI::x)
			.field("y", &ZXing::PointI::y)
			;

	value_object<ZXing::Position>("Position")
			.field("topLeft", emscripten::index<0>())
			.field("topRight", emscripten::index<1>())
			.field("bottomRight", emscripten::index<2>())
			.field("bottomLeft", emscripten::index<3>())
			;

	function("readBarcodeFromImage", &readBarcodeFromImage);
	function("readBarcodeFromPixmap", &readBarcodeFromPixmap);

	// obsoletes [[deprecated]]
	function("readBarcode", &readBarcodeFromImage);
	function("readBarcodeFromPng", &readBarcodeFromImage);
}
