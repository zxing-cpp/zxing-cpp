/*
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "BitArray.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QRVersion.h"
#include "pdf417/PDFDecoder.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

using namespace ZXing;

namespace ZXing::Aztec {
DecoderResult Decode(const BitArray& bits);
}

namespace ZXing::DataMatrix::DecodedBitStreamParser {
DecoderResult Decode(ByteArray&& bytes, const bool isDMRE);
}

namespace ZXing::QRCode {
DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel);
}

void az(const uint8_t* data, size_t size)
{
	BitArray bits;
	for (size_t i = 1; i < size - 1; ++i)
		bits.appendBits(data[i], 8);

	bits.appendBits(data[size - 1], (data[0] & 0x7) + 1);

	Aztec::Decode(bits);
}

void dm(const uint8_t* data, size_t size)
{
	ByteArray ba;
	ba.insert(ba.begin(), data, data + size);
	try {
		DataMatrix::DecodedBitStreamParser::Decode(std::move(ba), false);
	} catch (...) {
	}
}

void qr(const uint8_t* data, size_t size)
{
	auto version = QRCode::Version::Model2(std::clamp(data[0] & 0x3F, 1, 40));
	auto ecLevel = QRCode::ECLevelFromBits(data[0] >> 6);

	ByteArray ba;
	ba.insert(ba.begin(), data, data + size);

	QRCode::DecodeBitStream(std::move(ba), *version, ecLevel);
}

void pd(const uint8_t* data, size_t size)
{
	auto codewords = std::vector<int>(size / 2);
	auto u16 = reinterpret_cast<const uint16_t*>(data);

	for (int i = 0; i < Size(codewords); ++i)
		codewords[i] = u16[i] % 929;

	codewords[0] = Size(codewords);

	Pdf417::Decode(codewords);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 3)
		return 0;

	az(data, size);
	dm(data, size);
	qr(data, size);
	pd(data, size);

	return 0;
}
