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
#include "gtest/gtest.h"
#include "qrcode/QRDataMask.h"
#include "qrcode/QRDecoder.h"
#include "qrcode/QRVersion.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "ByteArray.h"
#include "DecoderResult.h"

namespace ZXing {
	namespace QRCode {
		DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel, const std::string& hintedCharset);
	}
}

using namespace ZXing;
using namespace ZXing::QRCode;

namespace {

	/**
	* Class that lets one easily build an array of bytes by appending bits at a time.
	*
	* @author Sean Owen
	*/
	class BitSourceBuilder {

		ByteArray _output;
		int _nextByte = 0;
		int _bitsLeftInNextByte = 8;

	public:
		void write(int value, int numBits) {
			if (numBits <= _bitsLeftInNextByte) {
				_nextByte <<= numBits;
				_nextByte |= value;
				_bitsLeftInNextByte -= numBits;
				if (_bitsLeftInNextByte == 0) {
					_output.push_back(static_cast<uint8_t>(_nextByte));
					_nextByte = 0;
					_bitsLeftInNextByte = 8;
				}
			}
			else {
				int bitsToWriteNow = _bitsLeftInNextByte;
				int numRestOfBits = numBits - bitsToWriteNow;
				int mask = 0xFF >> (8 - bitsToWriteNow);
				int valueToWriteNow = (value >> numRestOfBits) & mask;
				write(valueToWriteNow, bitsToWriteNow);
				write(value, numRestOfBits);
			}
		}

		ByteArray toByteArray() {
			if (_bitsLeftInNextByte < 8) {
				write(0, _bitsLeftInNextByte);
			}
			return _output;
		}
	};
}

TEST(QRDecodedBitStreamParserTest, SimpleByteMode)
{
    BitSourceBuilder builder;
    builder.write(0x04, 4); // Byte mode
    builder.write(0x03, 8); // 3 bytes
    builder.write(0xF1, 8);
    builder.write(0xF2, 8);
    builder.write(0xF3, 8);
    auto result = DecodeBitStream(builder.toByteArray(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
    EXPECT_EQ(L"\xF1\xF2\xF3", result);
}

TEST(QRDecodedBitStreamParserTest, SimpleSJIS)
{
    BitSourceBuilder builder;
    builder.write(0x04, 4); // Byte mode
    builder.write(0x04, 8); // 4 bytes
    builder.write(0xA1, 8);
    builder.write(0xA2, 8);
    builder.write(0xA3, 8);
    builder.write(0xD0, 8);
	auto result = DecodeBitStream(builder.toByteArray(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\uff61\uff62\uff63\uff90", result);
}

TEST(QRDecodedBitStreamParserTest, ECI)
{
    BitSourceBuilder builder;
    builder.write(0x07, 4); // ECI mode
    builder.write(0x02, 8); // ECI 2 = CP437 encoding
    builder.write(0x04, 4); // Byte mode
    builder.write(0x03, 8); // 3 bytes
    builder.write(0xA1, 8);
    builder.write(0xA2, 8);
    builder.write(0xA3, 8);
	auto result = DecodeBitStream(builder.toByteArray(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\xED\xF3\xFA", result);
}

TEST(QRDecodedBitStreamParserTest, Hanzi)
{
    BitSourceBuilder builder;
    builder.write(0x0D, 4); // Hanzi mode
    builder.write(0x01, 4); // Subset 1 = GB2312 encoding
    builder.write(0x01, 8); // 1 characters
    builder.write(0x03C1, 13);
	auto result = DecodeBitStream(builder.toByteArray(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\u963f", result);
}

TEST(QRDecodedBitStreamParserTest, HanziLevel1)
{
	BitSourceBuilder builder;
	builder.write(0x0D, 4); // Hanzi mode
	builder.write(0x01, 4); // Subset 1 = GB2312 encoding
	builder.write(0x01, 8); // 1 characters
	// A5A2 (U+30A2) => A5A2 - A1A1 = 401, 4*60 + 01 = 0181
	builder.write(0x0181, 13);
	auto result = DecodeBitStream(builder.toByteArray(), *Version::VersionForNumber(1), ErrorCorrectionLevel::Medium, "").text();
	EXPECT_EQ(L"\u30a2", result);
}
