// SPDX-License-Identifier: Apache-2.0

#include "wxcode/WXDecoder.h"

#include "BitArray.h"
#include "WXTestUtility.h"
#include "wxcode/WXFormatInformation.h"

#include "gtest/gtest.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using namespace ZXing;
using namespace ZXing::WXCode;

TEST(WXCodeDecoderTest, CorrectableErrorsAcrossVersionsAndProfiles)
{
	std::mt19937 rng(844);
	unsigned cases = 0;
	for (unsigned version = 0; version < 4; ++version) {
		std::vector<std::pair<unsigned, unsigned>> positions;
		for (unsigned angle = 0; angle < 72; ++angle)
			for (unsigned radius = 0; radius < 16; ++radius)
				if (VersionDataMask(version)[angle] & (1U << radius))
					positions.emplace_back(angle, radius);
		for (unsigned ec = 0; ec < 4; ++ec) {
			SCOPED_TRACE(::testing::Message() << "version=" << version << " ec=" << ec);
			ByteArray payload{static_cast<uint8_t>(0x41 + version), 0,
							  static_cast<uint8_t>(0xf0 + ec),      0x55,
							  static_cast<uint8_t>(version),        static_cast<uint8_t>(ec)};
			auto original = WXCode::Test::Encode(payload, version, ec);
			const auto& profile = *VersionEC(version, ec);
			for (unsigned damage = 0; damage <= profile.ecCodewords / 2; ++damage) {
				SCOPED_TRACE(::testing::Message() << "damaged symbols per block=" << damage);
				auto grid = original;
				for (unsigned block = 0; block < profile.blockCount; ++block) {
					std::vector<unsigned> symbols(profile.dataCodewords + profile.ecCodewords);
					std::iota(symbols.begin(), symbols.end(), 0);
					std::shuffle(symbols.begin(), symbols.end(), rng);
					for (unsigned i = 0; i < damage; ++i) {
						auto [angle, radius] = positions[8 * (symbols[i] * profile.blockCount + block) + rng() % 8];
						grid[angle][radius] ^= 1;
					}
				}
				EXPECT_EQ(Decode(grid), payload);
				++cases;
			}
		}
	}
	EXPECT_EQ(cases, 145U);
}

namespace {

ByteArray DecodeFields(std::initializer_list<std::pair<int, int>> fields)
{
	BitArray bits;
	for (auto [value, width] : fields)
		bits.appendBits(value, width);
	while (bits.size() < 28 * 8)
		bits.appendBit(false);
	return Decode(WXCode::Test::EncodeData(bits.toBytes(), 0, 0));
}

} // namespace

TEST(WXCodeDecoderTest, SegmentModes)
{
	EXPECT_EQ(DecodeFields({{1, 4}, {5, 10}, {123, 10}, {4, 7}, {4, 4}, {2, 8}, {0, 8}, {255, 8}}),
			  ByteArray({'1', '2', '3', '0', '4', 0, 255}));
	EXPECT_EQ(DecodeFields({{3, 4}, {0xabcd, 16}, {7, 4}, {26, 8}, {4, 4}, {2, 8}, {0, 8}, {255, 8}}), ByteArray({0, 255}));
	EXPECT_EQ(DecodeFields({{7, 4}, {0x80, 8}, {170, 8}, {4, 4}, {2, 8}, {0, 8}, {255, 8}}), ByteArray({0, 255}));
	EXPECT_EQ(DecodeFields({{8, 4}, {2, 8}, {0, 13}, {31 * 192, 13}}), ByteArray({0x81, 0x40, 0xe0, 0x40}));
	EXPECT_EQ(DecodeFields({{13, 4}, {1, 4}, {2, 8}, {0, 13}, {4 * 96, 13}}), ByteArray({0xa1, 0xa1, 0xaa, 0xa1}));
	EXPECT_EQ(DecodeFields({{5, 4}, {2, 4}, {2, 9}, {10 * 45 + 38, 11}}), ByteArray({'A', 29}));
	EXPECT_EQ(DecodeFields({{2, 4}, {2, 9}, {0, 11}}), ByteArray({'0', '0'}));
}

TEST(WXCodeDecoderTest, InvalidSegmentsAndGrid)
{
	EXPECT_TRUE(DecodeFields({{7, 4}, {14, 8}, {4, 4}, {2, 8}, {0, 8}, {255, 8}}).empty());
	EXPECT_TRUE(DecodeFields({{1, 4}, {1, 10}, {11, 4}}).empty());
	EXPECT_TRUE(DecodeFields({{15, 4}}).empty());
	auto grid = WXCode::Test::Encode(ByteArray{'A', 0, 0xf0, 0x55, 0, 0});
	grid.back().back() = 2;
	EXPECT_TRUE(Decode(grid).empty());
}

TEST(WXCodeDecoderTest, HanziUnsupportedSubsetConsumesOnlyHeader)
{
	EXPECT_EQ(DecodeFields({{13, 4}, {2, 4}, {200, 8}, {4, 4}, {1, 8}, {0xff, 8}}), ByteArray({0xff}));
}

TEST(WXCodeDecoderTest, AlphanumericConversionIncludesEarlierSegments)
{
	// Numeric '1' followed by alphanumeric 'A': base-45 digits {1, 10} map to target digit 55 ('T').
	EXPECT_EQ(DecodeFields({{1, 4}, {1, 10}, {1, 4}, {2, 4}, {1, 9}, {10, 6}}), ByteArray({'T'}));
}

TEST(WXCodeDecoderTest, FNC1EscapesOnlyCurrentAlphanumericSegment)
{
	EXPECT_EQ(DecodeFields({{4, 4}, {1, 8}, {'%', 8}, {5, 4}, {2, 4}, {5, 9}, {10 * 45 + 38, 11}, {38 * 45 + 11, 11}, {38, 6}}),
			  ByteArray({'%', 'A', '%', 'B', 29}));
}

TEST(WXCodeDecoderTest, ByteCountClampsToRemainingCapacity)
{
	BitArray bits;
	bits.appendBits(4, 4);
	bits.appendBits(24, 8);
	for (int i = 0; i < 24; ++i)
		bits.appendBits(0xff, 8);
	bits.appendBits(4, 4);
	bits.appendBits(255, 8);
	bits.appendBits(0xab, 8);
	ByteArray expected;
	expected.assign(24, 0xff);
	expected.push_back(0xab);
	EXPECT_EQ(Decode(WXCode::Test::EncodeData(bits.toBytes(), 0, 0)), expected);
}

TEST(WXCodeDecoderTest, ByteModeAcceptsInitiallyShortByte)
{
	BitArray bits;
	bits.appendBits(1, 4);
	bits.appendBits(58, 10);
	for (int i = 0; i < 19; ++i)
		bits.appendBits(0, 10);
	bits.appendBits(0, 4);
	bits.appendBits(4, 4);
	bits.appendBits(255, 8);
	bits.appendBits(0xa, 4);
	ByteArray expected;
	expected.assign(58, '0');
	expected.push_back(0xa);
	EXPECT_EQ(Decode(WXCode::Test::EncodeData(bits.toBytes(), 0, 0)), expected);
}

TEST(WXCodeDecoderTest, ByteModeRejectsLaterShortByte)
{
	// After the 12-bit header, 26 complete bytes and four bits remain.
	EXPECT_TRUE(DecodeFields({{4, 4}, {27, 8}}).empty());
}

TEST(WXCodeDecoderTest, ECIRejectsUnsupportedAssignmentsAndPrefixes)
{
	for (int assignment : {19, 31, 224}) {
		SCOPED_TRACE(assignment);
		EXPECT_TRUE(DecodeFields({{4, 4}, {1, 8}, {0xff, 8}, {7, 4}, {assignment, 8}}).empty());
	}
}

TEST(WXCodeDecoderTest, ECIThreeByteAssignmentPreservesPayload)
{
	EXPECT_EQ(DecodeFields({{7, 4}, {0xc0, 8}, {170, 16}, {4, 4}, {2, 8}, {0, 8}, {0xff, 8}}), ByteArray({0, 0xff}));
}

TEST(WXCodeDecoderTest, ECIRejectsTruncatedHeader)
{
	for (int firstByte : {-1, 0x80, 0xc0}) {
		SCOPED_TRACE(firstByte);
		// Fill the codeword capacity up to the ECI first byte or its continuation.
		int prefixLength = firstByte < 0 ? 26 : 25;
		BitArray bits;
		bits.appendBits(4, 4);
		bits.appendBits(prefixLength, 8);
		for (int i = 0; i < prefixLength; ++i)
			bits.appendBits(0xff, 8);
		bits.appendBits(7, 4);
		if (firstByte >= 0)
			bits.appendBits(firstByte, 8);
		ASSERT_EQ(bits.size(), 28 * 8);
		EXPECT_TRUE(Decode(WXCode::Test::EncodeData(bits.toBytes(), 0, 0)).empty());
	}
}
