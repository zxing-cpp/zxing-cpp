/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

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
	std::wstring text;
	std::string error;
	ZXing::Position position;
};

ReadResult readBarcodeFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setTryDownscale(tryHarder);
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
			return { ToString(result.format()), result.text(), "", result.position() };
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
		hints.setTryDownscale(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));

		auto result =
			ReadBarcode({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBX}, hints);

		if (result.isValid()) {
			return { ToString(result.format()), result.text(), "", result.position() };
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

	// obsoletes
	function("readBarcode", &readBarcodeFromImage);
	function("readBarcodeFromPng", &readBarcodeFromImage);
}
