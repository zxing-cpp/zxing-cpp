/*
 * Copyright 2016 Nu-book Inc.
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <memory>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace ZXing;

struct ReadResult
{
	std::string format{};
	std::string text{};
	emscripten::val bytes;
	std::string error{};
	Position position{};
	std::string symbologyIdentifier{};
};

std::vector<ReadResult> readBarcodes(ImageView iv, bool tryHarder, const std::string& format, int maxSymbols)
{
	try {
		ReaderOptions opts;
		opts.setTryHarder(tryHarder);
		opts.setTryRotate(tryHarder);
		opts.setTryInvert(tryHarder);
		opts.setTryDownscale(tryHarder);
		opts.setFormats(BarcodeFormatsFromString(format));
		opts.setMaxNumberOfSymbols(maxSymbols);
//		opts.setReturnErrors(maxSymbols > 1);

		auto barcodes = ReadBarcodes(iv, opts);

		std::vector<ReadResult> readResults{};
		readResults.reserve(barcodes.size());

		thread_local const emscripten::val Uint8Array = emscripten::val::global("Uint8Array");

		for (auto&& barcode : barcodes) {
			const ByteArray& bytes = barcode.bytes();
			readResults.push_back({
				ToString(barcode.format()),
				barcode.text(),
				Uint8Array.new_(emscripten::typed_memory_view(bytes.size(), bytes.data())),
				ToString(barcode.error()),
				barcode.position(),
				barcode.symbologyIdentifier()
			});
		}

		return readResults;
	} catch (const std::exception& e) {
		return {{"", "", {}, e.what()}};
	} catch (...) {
		return {{"", "", {}, "Unknown error"}};
	}
	return {};
}

std::vector<ReadResult> readBarcodesFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format, int maxSymbols)
{
	int width, height, channels;
	std::unique_ptr<stbi_uc, void (*)(void*)> buffer(
		stbi_load_from_memory(reinterpret_cast<const unsigned char*>(bufferPtr), bufferLength, &width, &height, &channels, 1),
		stbi_image_free);
	if (buffer == nullptr)
		return {{"", "", {}, "Error loading image"}};

	return readBarcodes({buffer.get(), width, height, ImageFormat::Lum}, tryHarder, format, maxSymbols);
}

ReadResult readBarcodeFromImage(int bufferPtr, int bufferLength, bool tryHarder, std::string format)
{
	return FirstOrDefault(readBarcodesFromImage(bufferPtr, bufferLength, tryHarder, format, 1));
}

std::vector<ReadResult> readBarcodesFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format, int maxSymbols)
{
	return readBarcodes({reinterpret_cast<uint8_t*>(bufferPtr), imgWidth, imgHeight, ImageFormat::RGBA}, tryHarder, format, maxSymbols);
}

ReadResult readBarcodeFromPixmap(int bufferPtr, int imgWidth, int imgHeight, bool tryHarder, std::string format)
{
	return FirstOrDefault(readBarcodesFromPixmap(bufferPtr, imgWidth, imgHeight, tryHarder, format, 1));
}

EMSCRIPTEN_BINDINGS(BarcodeReader)
{
	using namespace emscripten;

	value_object<ReadResult>("ReadResult")
		.field("format", &ReadResult::format)
		.field("text", &ReadResult::text)
		.field("bytes", &ReadResult::bytes)
		.field("error", &ReadResult::error)
		.field("position", &ReadResult::position)
		.field("symbologyIdentifier", &ReadResult::symbologyIdentifier);

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
