/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2011 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "oned/ODCodabarWriter.h"
#include "BitMatrixIO.h"
#include "ReaderOptions.h"
#include "Barcode.h"
#include "oned/ODCodabarReader.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace ZXing;
using namespace ZXing::OneD;

namespace {
	std::string Encode(const std::string& input)
	{
		auto result = ToString(CodabarWriter().encode(input, 0, 0), '1', '0', false);
		return result.substr(0, result.size() - 1);	// remove the \n at the end
	}
}

TEST(ODCodaBarWriterTest, Encode)
{
	EXPECT_EQ(Encode("B515-3/B"),
				  "00000"
				  "1001001011" "0110101001" "0101011001" "0110101001" "0101001101"
				  "0110010101" "01101101011" "01001001011"
				  "00000");
}

TEST(ODCodaBarWriterTest, Encode2)
{
	EXPECT_EQ(Encode("T123T"),
			  "00000"
			  "1011001001" "0101011001" "0101001011" "0110010101" "01011001001"
			  "00000");
}

TEST(ODCodaBarWriterTest, AltStartEnd)
{
	EXPECT_EQ(Encode("T123456789-$T"), Encode("A123456789-$A"));
}

TEST(ODCodaBarWriterTest, FullCircle)
{
	std::string text = "A0123456789-$:/.+A";
	auto matrix = CodabarWriter().encode(text, 0, 0);
	auto opts = ReaderOptions();

	auto res = OneD::DecodeSingleRow(CodabarReader(opts), matrix.row(0));
	EXPECT_EQ(text, res.text());
}

TEST(ODCodaBarWriterTest, InvalidChars)
{
	EXPECT_THROW({Encode("AxA");}, std::invalid_argument );
	EXPECT_THROW({Encode("a0a");}, std::invalid_argument );
}
