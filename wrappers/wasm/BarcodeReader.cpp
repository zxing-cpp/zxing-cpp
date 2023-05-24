/*
 * Copyright 2016 Nu-book Inc.
 */
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include <emscripten/bind.h>
#include <memory>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct ReadResult
{
	std::string format{};
	std::string text{};
	std::string error{};
	ZXing::Position position{};
};

std::vector<ReadResult> readBarcodesFromImageView(ZXing::ImageView iv, bool tryHarder, const std::string& format,
												  const int maxSymbols = 0xff)
{
	using namespace ZXing;
	try {
		DecodeHints hints;
		hints.setTryHarder(tryHarder);
		hints.setTryRotate(tryHarder);
		hints.setTryInvert(tryHarder);
		hints.setTryDownscale(tryHarder);
		hints.setFormats(BarcodeFormatsFromString(format));
		hints.setMaxNumberOfSymbols(maxSymbols);

		auto results = ReadBarcodes(iv, hints);

		std::vector<ReadResult> readResults{};
		readResults.reserve(results.size());

		for (auto& result : results) {
			readResults.push_back({ToString(result.format()), result.text(), {}, result.position()});
		}

		return readResults;
	} catch (const std::exception& e) {
		return {{"", "", e.what()}};
	} catch (...) {
		return {{"", "", "Unknown error"}};
	}
	return {};
}

std::vector<ReadResult> readBarcodesFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format,
											  const int maxSymbols)
{
	using namespace ZXing;

	int width, height, channels;
	std::unique_ptr<stbi_uc, void (*)(void*)> buffer(
		stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bufferPtr), bufferLength, &width, &height, &channels, 1),
		stbi_image_free);
	if (buffer == nullptr)
		return {{"", "", "Error loading image"}};

	return readBarcodesFromImageView({buffer.get(), width, height, ImageFormat::Lum}, tryHarder, format, maxSymbols);
}

ReadResult readBarcodeFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
	using namespace ZXing;
	auto results = readBarcodesFromImage(bufferPtr, bufferLength, tryHarder, format, 1);
	return results.empty() ? ReadResult() : results.front();
}

std::vector<ReadResult> readBarcodesFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format,
											   const int maxSymbols)
{
	using namespace ZXing;
	return readBarcodesFromImageView({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBX}, tryHarder,
									 format, maxSymbols);
}

ReadResult readBarcodeFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format)
{
	using namespace ZXing;
	auto results = readBarcodesFromPixmap(bufferPtr, imgWidth, imgHeight, tryHarder, format, 1);
	return results.empty() ? ReadResult() : results.front();
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
	using namespace emscripten;

	value_object<ReadResult>("ReadResult")
		.field("format", &ReadResult::format)
		.field("text", &ReadResult::text)
		.field("error", &ReadResult::error)
		.field("position", &ReadResult::position);

	value_object<ZXing::PointI>("Point").field("x", &ZXing::PointI::x).field("y", &ZXing::PointI::y);

	value_object<ZXing::Position>("Position")
		.field("topLeft", emscripten::index<0>())
		.field("topRight", emscripten::index<1>())
		.field("bottomRight", emscripten::index<2>())
		.field("bottomLeft", emscripten::index<3>());

	register_vector<ReadResult>("vector<ReadResult>");

	function("readBarcodeFromImage", &readBarcodeFromImage);
	function("readBarcodeFromPixmap", &readBarcodeFromPixmap);

	function("readBarcodesFromImage", &readBarcodesFromImage);
	function("readBarcodesFromPixmap", &readBarcodesFromPixmap);
};
