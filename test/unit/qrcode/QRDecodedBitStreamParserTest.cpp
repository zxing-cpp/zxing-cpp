/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
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

#include "BitArray.h"
#include "ByteArray.h"
#include "DecoderResult.h"
#include "qrcode/QRDataMask.h"
#include "qrcode/QRDecoder.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QRVersion.h"

#include "gtest/gtest.h"

namespace ZXing {
	namespace QRCode {
		DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel, const std::string& hintedCharset);
	}
}

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRDecodedBitStreamParserTest, SimpleByteMode)
{
    BitArray ba;
    ba.appendBits(0x04, 4); // Byte mode
    ba.appendBits(0x03, 8); // 3 bytes
    ba.appendBits(0xF1, 8);
    ba.appendBits(0xF2, 8);
    ba.appendBits(0xF3, 8);
    auto result = DecodeBitStream(ba.toBytes(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
    EXPECT_EQ(L"\xF1\xF2\xF3", result);
}

TEST(QRDecodedBitStreamParserTest, SimpleSJIS)
{
    BitArray ba;
    ba.appendBits(0x04, 4); // Byte mode
    ba.appendBits(0x04, 8); // 4 bytes
    ba.appendBits(0xA1, 8);
    ba.appendBits(0xA2, 8);
    ba.appendBits(0xA3, 8);
    ba.appendBits(0xD0, 8);
	auto result = DecodeBitStream(ba.toBytes(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\uff61\uff62\uff63\uff90", result);
}

TEST(QRDecodedBitStreamParserTest, ECI)
{
    BitArray ba;
    ba.appendBits(0x07, 4); // ECI mode
    ba.appendBits(0x02, 8); // ECI 2 = CP437 encoding
    ba.appendBits(0x04, 4); // Byte mode
    ba.appendBits(0x03, 8); // 3 bytes
    ba.appendBits(0xA1, 8);
    ba.appendBits(0xA2, 8);
    ba.appendBits(0xA3, 8);
	auto result = DecodeBitStream(ba.toBytes(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\xED\xF3\xFA", result);
}

TEST(QRDecodedBitStreamParserTest, Hanzi)
{
    BitArray ba;
    ba.appendBits(0x0D, 4); // Hanzi mode
    ba.appendBits(0x01, 4); // Subset 1 = GB2312 encoding
    ba.appendBits(0x01, 8); // 1 characters
    ba.appendBits(0x03C1, 13);
	auto result = DecodeBitStream(ba.toBytes(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\u963f", result);
}

TEST(QRDecodedBitStreamParserTest, HanziLevel1)
{
	BitArray ba;
	ba.appendBits(0x0D, 4); // Hanzi mode
	ba.appendBits(0x01, 4); // Subset 1 = GB2312 encoding
	ba.appendBits(0x01, 8); // 1 characters
	// A5A2 (U+30A2) => A5A2 - A1A1 = 401, 4*60 + 01 = 0181
	ba.appendBits(0x0181, 13);

	auto result = DecodeBitStream(ba.toBytes(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\u30a2", result);
}
