/*
* Copyright 2022 gitlost
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCode39Reader.h"

#include "Barcode.h"
#include "ReaderOptions.h"
#include "ZXAlgorithms.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::OneD;

// Helper to call decodePattern()
static Barcode parse(PatternRow row, const ReaderOptions& opts = {})
{
	Code39Reader reader(opts);

	row.insert(row.begin(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });
	row.insert(row.end(), { 0, 1, 2, 1, 1, 2, 1, 2, 1, 1, 0 });

	std::unique_ptr<RowReader::DecodingState> state;
	PatternView next(row);
	return reader.decodePattern(0, next, state);
}

TEST(ODCode39ReaderTest, SymbologyIdentifier)
{
	{
		// Plain "A"
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "A");
	}
	{
		// "A" with checksum
		PatternRow row({ 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A1");
		EXPECT_EQ(result.text(), "AA");
	}
	{
		// Extended "a"
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A4");
		EXPECT_EQ(result.text(), "a");

		result = parse(row, ReaderOptions().formats(BarcodeFormat::Code39Std));
		EXPECT_EQ(result.symbologyIdentifier(), "]A0");
		EXPECT_EQ(result.text(), "+A");
	}
	{
		// Extended "a" with checksum
		PatternRow row({ 1, 2, 1, 1, 1, 2, 1, 2, 1, 0, 2, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 2, 1, 1, 2, 1, 1 });
		auto result = parse(row);
		EXPECT_EQ(result.symbologyIdentifier(), "]A5");
		EXPECT_EQ(result.text(), "a8");

		result = parse(row, ReaderOptions().formats(BarcodeFormat::Code39Std));
		EXPECT_EQ(result.symbologyIdentifier(), "]A1");
		EXPECT_EQ(result.text(), "+A8");
	}
}

TEST(ODCode39ReaderTest, Code32)
{
	auto parse32 = [](PatternRow row) {
		return parse(row, ReaderOptions().formats(BarcodeFormat::Code32 | BarcodeFormat::Code39));
	};

	static constexpr char ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%*";
	static constexpr int CHARACTER_ENCODINGS[] = {
		0x034, 0x121, 0x061, 0x160, 0x031, 0x130, 0x070, 0x025, 0x124, 0x064, // 0-9
		0x109, 0x049, 0x148, 0x019, 0x118, 0x058, 0x00D, 0x10C, 0x04C, 0x01C, // A-J
		0x103, 0x043, 0x142, 0x013, 0x112, 0x052, 0x007, 0x106, 0x046, 0x016, // K-T
		0x181, 0x0C1, 0x1C0, 0x091, 0x190, 0x0D0, 0x085, 0x184, 0x0C4, 0x0A8, // U-$
		0x0A2, 0x08A, 0x02A, 0x094 // /-% , *
	};

	auto encode = [&](std::string_view chars) {
		PatternRow row;
		for (char c : chars) {
			int i = IndexOf(ALPHABET, c);
			int pattern = CHARACTER_ENCODINGS[i];
			if (!row.empty()) row.push_back(1);
			for (int j = 8; j >= 0; --j)
				row.push_back((pattern >> j) & 1 ? 2 : 1);
		}
		return row;
	};

	{
		// Valid Code 32: "000000" in TABELLA (all '0's)
		auto row = encode("000000");
		auto result = parse32(row);
		EXPECT_EQ(result.format(), BarcodeFormat::Code32);
		EXPECT_EQ(result.text(), "A000000000");
	}
	{
		// Invalid Code 32: "ZZZZZZ" in TABELLA
		// val = 32^6 - 1 = 1,073,741,823 > 999,999,999
		auto row = encode("ZZZZZZ");
		auto result = parse32(row);
		// Should fall back to Code 39
		EXPECT_EQ(result.format(), BarcodeFormat::Code39);
		EXPECT_EQ(result.text(), "ZZZZZZ");
	}
}
