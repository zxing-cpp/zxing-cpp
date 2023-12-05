/*
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <stdint.h>
#include <stddef.h>

#include "ByteArray.h"
#include "DecoderResult.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QRVersion.h"

using namespace ZXing;

namespace ZXing::QRCode {
DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 3)
		return 0;

	auto version = QRCode::Version::Model2(std::clamp(data[0] & 0x3F, 1, 40));
	auto ecLevel = QRCode::ECLevelFromBits(data[0] >> 6);

	ByteArray ba;
	ba.insert(ba.begin(), data, data + size);

	QRCode::DecodeBitStream(std::move(ba), *version, ecLevel);

	return 0;
}
