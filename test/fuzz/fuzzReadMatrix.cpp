/*
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ReadBarcode.h"

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

using namespace ZXing;

uint64_t Expand(uint8_t b)
{
    uint64_t shift = 0x0000040810204081ul; // bits set: 0, 7, 14, 21, 28, 35, 42
    uint64_t mask = 0x0001010101010101ul; // bits set: 0, 8, 16, 24, 32, 40, 48
    return ((uint64_t)(b & 127) * shift & mask) | (uint64_t)(b & 128) << 49;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 3)
		return 0;

	static auto opts = ReaderOptions()
						   .setFormats(BarcodeFormat::MatrixCodes)
						   .setBinarizer(Binarizer::BoolCast)
						   .setReturnErrors(true)
						   .setTryInvert(false)
						   .setTryRotate(false);

	int ratio = data[0] + 1;
	int nBits = (size - 1) * 8;
	int width = std::clamp(nBits * ratio / 256, 1, nBits);
	int height = std::clamp(nBits / width, 1, nBits);

	assert(width * height <= nBits);

	ByteArray buffer(nBits);
	for (size_t i = 1; i < size; ++i)
		*reinterpret_cast<uint64_t*>(&buffer[(i - 1) * 8]) = Expand(data[i]);

#ifdef PRINT_DEBUG
	printf("s: %zu, r: %d, n: %d -> %d x %d\n", size, ratio, nBits, width, height);
#endif

	auto image = ImageView(buffer.data(), width, height, ImageFormat::Lum);
	auto res = ReadBarcodes(image, opts);

#ifdef PRINT_DEBUG
	for (const auto& r : res)
		printf("%s: %s / %s\n", ToString(r.format()).c_str(), r.text().c_str(), ToString(r.error()).c_str());
#endif

	static int detectedSybols = 0;
	detectedSybols += Size(res);
	if (!res.empty() && detectedSybols % 100 == 0)
		printf("detected barcode symbols: %d\n", detectedSybols);

	return 0;
}
