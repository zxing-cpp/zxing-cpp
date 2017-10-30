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
#include "qrcode/QRCodecMode.h"
#include "qrcode/QRVersion.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRModeTest, ForBits)
{
    ASSERT_EQ(CodecMode::TERMINATOR, CodecMode::ModeForBits(0x00));
    ASSERT_EQ(CodecMode::NUMERIC, CodecMode::ModeForBits(0x01));
    ASSERT_EQ(CodecMode::ALPHANUMERIC, CodecMode::ModeForBits(0x02));
    ASSERT_EQ(CodecMode::BYTE, CodecMode::ModeForBits(0x04));
    ASSERT_EQ(CodecMode::KANJI, CodecMode::ModeForBits(0x08));
	ASSERT_THROW(CodecMode::ModeForBits(0x10), std::invalid_argument);
}

TEST(QRModeTest, CharacterCount)
{
    // Spot check a few values
    ASSERT_EQ(10, CodecMode::CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(5)));
    ASSERT_EQ(12, CodecMode::CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(26)));
    ASSERT_EQ(14, CodecMode::CharacterCountBits(CodecMode::NUMERIC, *Version::VersionForNumber(40)));
    ASSERT_EQ(9, CodecMode::CharacterCountBits(CodecMode::ALPHANUMERIC, *Version::VersionForNumber(6)));
    ASSERT_EQ(8, CodecMode::CharacterCountBits(CodecMode::BYTE, *Version::VersionForNumber(7)));
    ASSERT_EQ(8, CodecMode::CharacterCountBits(CodecMode::KANJI, *Version::VersionForNumber(8)));
}
