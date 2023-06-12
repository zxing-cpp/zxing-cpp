/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2006 Jeremias Maerki
*/
// SPDX-License-Identifier: Apache-2.0

#include "datamatrix/DMSymbolInfo.h"

#include "gtest/gtest.h"

using namespace ZXing::DataMatrix;

TEST(DMSymbolInfoTest, SymbolInfo)
{
	auto info = SymbolInfo::Lookup(3);
	EXPECT_EQ(5, info->errorCodewords());
	EXPECT_EQ(8, info->matrixWidth());
	EXPECT_EQ(8, info->matrixHeight());
	EXPECT_EQ(10, info->symbolWidth());
	EXPECT_EQ(10, info->symbolHeight());

	info = SymbolInfo::Lookup(3, SymbolShape::RECTANGLE);
	EXPECT_EQ(7, info->errorCodewords());
	EXPECT_EQ(16, info->matrixWidth());
	EXPECT_EQ(6, info->matrixHeight());
	EXPECT_EQ(18, info->symbolWidth());
	EXPECT_EQ(8, info->symbolHeight());

	info = SymbolInfo::Lookup(9);
	EXPECT_EQ(11, info->errorCodewords());
	EXPECT_EQ(14, info->matrixWidth());
	EXPECT_EQ(6, info->matrixHeight());
	EXPECT_EQ(32, info->symbolWidth());
	EXPECT_EQ(8, info->symbolHeight());

	info = SymbolInfo::Lookup(9, SymbolShape::SQUARE);
	EXPECT_EQ(12, info->errorCodewords());
	EXPECT_EQ(14, info->matrixWidth());
	EXPECT_EQ(14, info->matrixHeight());
	EXPECT_EQ(16, info->symbolWidth());
	EXPECT_EQ(16, info->symbolHeight());

	info = SymbolInfo::Lookup(1559);
	EXPECT_EQ(nullptr, info) << "There's no rectangular symbol for more than 1558 data codewords";

	info = SymbolInfo::Lookup(50, SymbolShape::RECTANGLE);
	EXPECT_EQ(nullptr, info) << "There's no rectangular symbol for 50 data codewords";

	info = SymbolInfo::Lookup(35);
	EXPECT_EQ(24, info->symbolWidth());
	EXPECT_EQ(24, info->symbolHeight());

	int minWidth = 26;
	int minHeight = 26;
	int maxWidth = 26;
	int maxHeight = 26;

	info = SymbolInfo::Lookup(35, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(26, info->symbolWidth());
	EXPECT_EQ(26, info->symbolHeight());

	info = SymbolInfo::Lookup(45, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(nullptr, info);

	maxWidth = 32;
	maxHeight = 32;

	info = SymbolInfo::Lookup(35, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(26, info->symbolWidth());
	EXPECT_EQ(26, info->symbolHeight());

	info = SymbolInfo::Lookup(40, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(26, info->symbolWidth());
	EXPECT_EQ(26, info->symbolHeight());

	info = SymbolInfo::Lookup(45, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(32, info->symbolWidth());
	EXPECT_EQ(32, info->symbolHeight());

	info = SymbolInfo::Lookup(63, SymbolShape::NONE, minWidth, minHeight, maxWidth, maxHeight);
	EXPECT_EQ(nullptr, info);
}
