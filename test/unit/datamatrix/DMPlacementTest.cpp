/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2006 Jeremias Maerki
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrixIO.h"
#include "ByteArray.h"
#include "datamatrix/DMBitLayout.h"

#include "gtest/gtest.h"
#include <algorithm>
#include <iterator>
#include <sstream>

using namespace ZXing;
using namespace ZXing::DataMatrix;

namespace {

	ByteArray Unvisualize(const char* visualized) {
		std::istringstream input(visualized);
		ByteArray result;
		std::copy(std::istream_iterator<int>(input), std::istream_iterator<int>(), std::back_inserter(result));
		return result;
	}
}

TEST(DMPlacementTest, Placement)
{
		auto codewords = Unvisualize("66 74 78 66 74 78 129 56 35 102 192 96 226 100 156 1 107 221"); //"AIMAIM" encoded
		auto matrix = BitMatrixFromCodewords(codewords, 12, 12);
		std::string expected =
			"011100001111\n"
			"001010101000\n"
			"010001010100\n"
			"001010100010\n"
			"000111000100\n"
			"011000010100\n"
			"000100001101\n"
			"011000010000\n"
			"001100001101\n"
			"100010010111\n"
			"011101011010\n"
			"001011001010\n";
		EXPECT_EQ(expected, ToString(matrix, '1', '0', false));
}
