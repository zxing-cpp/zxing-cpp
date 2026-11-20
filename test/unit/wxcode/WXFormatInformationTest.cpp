// SPDX-License-Identifier: Apache-2.0

#include "wxcode/WXFormatInformation.h"

#include "gtest/gtest.h"
#include <array>
#include <bit>
#include <cstdint>

using namespace ZXing::WXCode;

TEST(WXCodeFormatTest, NativeMaskDigests)
{
	// Independent native mask digests; see the WX Code README for provenance.
	static constexpr std::array<uint32_t, 32> NATIVE_MASK_HASHES = {
		0xf1b73785U, 0xe4e1ea35U, 0x31783a45U, 0xcb3041c5U, 0xb81c9e85U, 0x99b937c5U, 0xdc71f8b5U, 0x1fd634c5U,
		0x7ff4cde5U, 0x280cae85U, 0xb7340b85U, 0x848a83f5U, 0x982abc25U, 0xfb2041c5U, 0x4b2c9755U, 0xabe1ae05U,
		0x77cfca85U, 0xf47e81c5U, 0x90e5b505U, 0xb88116c5U, 0x8f36ee05U, 0xd30331b4U, 0xd392cac5U, 0xbe736e05U,
		0x392951c5U, 0x74d425c5U, 0x318143c5U, 0x29d91cc5U, 0x2acf8335U, 0x67ba0cc5U, 0xc603ab85U, 0x5f4d9d35U};
	for (unsigned mask = 0; mask < 32; ++mask) {
		uint32_t hash = 2166136261U;
		for (unsigned angle = 0; angle < 72; ++angle) {
			for (unsigned radius = 0; radius < 16; ++radius) {
				unsigned value = MaskBit(mask, angle, radius);
				ASSERT_TRUE(value <= 1);
				hash = (hash ^ value) * 16777619U;
			}
		}
		ASSERT_TRUE(hash == NATIVE_MASK_HASHES[mask]);
	}
	ASSERT_TRUE(MaskBit(32, 0, 0) == 0);
	ASSERT_TRUE(MaskBit(0, 72, 0) == 0);
	ASSERT_TRUE(MaskBit(0, 0, 16) == 0);
	ASSERT_TRUE(MaskBit(0, UINT32_MAX, UINT32_MAX) == 0);
}

TEST(WXCodeFormatTest, FormatCodebookAndCorrection)
{
	unsigned format_cells = 0;
	for (unsigned angle = 0; angle < 72; ++angle)
		format_cells += std::popcount(FormatMask()[angle]);
	ASSERT_TRUE(format_cells == 36);

	FormatInformation format;
	ASSERT_TRUE(FormatWord(0) == 70308);
	ASSERT_TRUE(FormatWord(192) == 0);
	ASSERT_TRUE(FormatWord(511) == 37463);
	for (unsigned i = 0; i < 512; ++i) {
		uint32_t word = FormatWord(i);
		ASSERT_TRUE(DecodeFormat(word, word, format));
		ASSERT_TRUE(format.version == i / 128);
		ASSERT_TRUE(format.ecBits == (i / 32) % 4);
		ASSERT_TRUE(format.mask == i % 32);
		ASSERT_TRUE(format.distance == 0);
	}
	/* The codebook's minimum separation is six bits. Three-bit errors
	   can tie, so use a captured word whose three flips select it uniquely. */
	for (unsigned error_bits = 1; error_bits <= 3; ++error_bits) {
		uint32_t corrupt = 70308U ^ ((1U << error_bits) - 1);
		ASSERT_TRUE(DecodeFormat(corrupt, corrupt, format));
		ASSERT_TRUE(format.version == 0 && format.ecBits == 0 && format.mask == 0);
		ASSERT_TRUE(format.distance == error_bits);
	}
	ASSERT_TRUE(DecodeFormat(0x4e, 70308, format));
	ASSERT_TRUE(format.version == 0 && format.ecBits == 0 && format.mask == 0);
	ASSERT_TRUE(!DecodeFormat(0x4e, 0x4e, format));
	ASSERT_TRUE(!DecodeFormat(0x40000, 0, format));
	ASSERT_TRUE(!DecodeFormat(0, UINT32_MAX, format));
}

TEST(WXCodeFormatTest, VersionCapacitiesAndProfiles)
{
	static constexpr std::array<unsigned, 4> NATIVE_COUNTER_COUNTS = {304, 416, 528, 416};
	for (unsigned version = 0; version < 4; ++version) {
		auto mask = VersionDataMask(version);
		unsigned cells = 0;
		ASSERT_EQ(mask.size(), 72U);
		for (unsigned angle = 0; angle < 72; ++angle) {
			ASSERT_TRUE(mask[angle] <= 0xffff);
			cells += std::popcount(mask[angle]);
		}
		ASSERT_TRUE(cells == NATIVE_COUNTER_COUNTS[version]);
		ASSERT_TRUE(VersionCounterCount(version) == cells);
		ASSERT_TRUE(VersionTotalCodewords(version) * 8 == cells);
		for (unsigned ec = 0; ec < 4; ++ec) {
			const ECProfile* profile = VersionEC(version, ec);
			ASSERT_TRUE(profile != nullptr);
			ASSERT_TRUE((profile->dataCodewords + profile->ecCodewords) * profile->blockCount == cells / 8);
		}
	}
	ASSERT_TRUE(VersionDataMask(4).empty());
	ASSERT_TRUE(VersionTotalCodewords(4) == 0);
	ASSERT_TRUE(VersionCounterCount(UINT32_MAX) == 0);
	ASSERT_TRUE(VersionEC(4, 0) == nullptr);
	ASSERT_TRUE(VersionEC(0, 4) == nullptr);
}

TEST(WXCodeFormatTest, EqualDistanceChoosesLowestCodebookIndex)
{
	// Entries 0 and 1 differ in six bits. Flipping three of them produces a tie.
	uint32_t midpoint = FormatWord(0) ^ 0x38U;
	ASSERT_EQ(std::popcount(midpoint ^ FormatWord(0)), 3);
	ASSERT_EQ(std::popcount(midpoint ^ FormatWord(1)), 3);
	FormatInformation format{};
	ASSERT_TRUE(DecodeFormat(midpoint, midpoint, format));
	EXPECT_EQ(format.version, 0U);
	EXPECT_EQ(format.ecBits, 0U);
	EXPECT_EQ(format.mask, 0U);
	EXPECT_EQ(format.distance, 3U);

	// Equally good copies also choose the lowest index, regardless of copy order.
	for (bool reverse : {false, true}) {
		ASSERT_TRUE(DecodeFormat(FormatWord(reverse ? 0 : 1), FormatWord(reverse ? 1 : 0), format));
		EXPECT_EQ(format.version, 0U);
		EXPECT_EQ(format.ecBits, 0U);
		EXPECT_EQ(format.mask, 0U);
		EXPECT_EQ(format.distance, 0U);
	}
}
