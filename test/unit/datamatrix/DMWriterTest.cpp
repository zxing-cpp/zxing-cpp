/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2008 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "datamatrix/DMWriter.h"
#include "BitMatrixIO.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::DataMatrix;

TEST(DMWriterTest, ImageWriter)
{
	int bigEnough = 64;
	Writer writer;
	writer.setMargin(0).setShapeHint(SymbolShape::SQUARE);
	auto matrix = writer.encode(L"Hello Google", bigEnough, bigEnough);
	EXPECT_LE(matrix.width(), bigEnough);
	EXPECT_LE(matrix.height(), bigEnough);
}

TEST(DMWriterTest, Writer2)
{
	int bigEnough = 14;
	Writer writer;
	writer.setMargin(0).setShapeHint(SymbolShape::SQUARE);
	auto matrix = writer.encode(L"Hello Me", bigEnough, bigEnough);
	EXPECT_EQ(matrix.width(), bigEnough);
	EXPECT_EQ(matrix.height(), bigEnough);
}

TEST(DMWriterTest, TooSmallSize)
{
	// The DataMatrix will not fit in this size, so the matrix should come back bigger
	int tooSmall = 8;
	Writer writer;
	writer.setMargin(0);
	auto matrix = writer.encode(L"http://www.google.com/", tooSmall, tooSmall);
	EXPECT_GT(matrix.width(), tooSmall);
	EXPECT_GT(matrix.height(), tooSmall);
}

static void DoTest(const std::wstring& text, SymbolShape shape, const char* expected)
{
	Writer writer;
	writer.setMargin(0).setShapeHint(shape);
	auto matrix = writer.encode(text, 0, 0);
	auto actual = ToString(matrix, 'X', ' ', true);
	EXPECT_EQ(expected, actual);
}

TEST(DMWriterTest, Small)
{
	DoTest(L"0", SymbolShape::SQUARE,
	       "X   X   X   X   X   \n"
	       "X X   X X     X   X \n"
	       "X       X X     X   \n"
	       "X     X           X \n"
	       "X     X   X X X X   \n"
	       "X X X X X X       X \n"
	       "X       X   X       \n"
	       "X X     X X X   X X \n"
	       "X   X       X       \n"
	       "X X X X X X X X X X \n" );
}

TEST(DMWriterTest, Rectangle)
{
	DoTest(L"abcde", SymbolShape::RECTANGLE,
	       "X   X   X   X   X   X   X   X   X   \n"
	       "X   X X     X     X     X   X X   X \n"
	       "X X       X X   X     X   X X       \n"
	       "X   X X X     X     X X   X X   X X \n"
	       "X     X X X   X X X X X X X X X     \n"
	       "X   X X     X     X X X X       X X \n"
	       "X X   X X X       X X X X X   X X   \n"
	       "X X X X X X X X X X X X X X X X X X \n");
}

TEST(DMWriterTest, Large)
{
	auto text = L"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-"
				L"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-"
				L"123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-";
	auto expected =
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   \n"
		"X X     X     X   X   X     X X X X     X     X   X X     X X       X X X   X   X   X     X X       X X \n"
		"X X                   X X X   X   X                 X X X X X   X X     X   X   X X   X X X X   X X     \n"
		"X X     X X X     X X     X   X   X     X X X X X X X   X   X     X             X X X   X   X     X   X \n"
		"X X       X   X   X               X       X     X   X   X   X     X X X X   X X     X   X   X           \n"
		"X   X   X     X X   X X X     X X   X   X X       X X   X X X X     X   X     X         X X X   X X X X \n"
		"X   X X   X X X       X   X   X X   X X   X X X X   X X   X   X     X   X     X X X X X   X       X X   \n"
		"X   X X X   X   X   X     X X       X X     X     X X X   X   X     X X X X     X     X   X           X \n"
		"X X     X   X   X X   X X X X   X X   X             X         X X X   X   X               X X   X X X   \n"
		"X X             X X X   X   X     X   X     X X X X X     X X     X   X   X     X X X X       X   X X X \n"
		"X X X X X   X X     X   X   X         X       X     X X   X               X       X           X X X     \n"
		"X   X   X     X         X X X     X X   X   X     X X X X   X X X     X X   X   X         X X X   X   X \n"
		"X   X   X     X X X X X   X   X   X X   X X   X X   X X       X   X   X X   X X   X   X     X   X X     \n"
		"X   X X X X     X     X       X X       X X X   X X X   X   X     X X       X     X X       X X X X   X \n"
		"X X   X   X               X X X X   X X     X   X   X   X X   X X X X   X X   X X X X   X   X     X X   \n"
		"X X   X   X     X X X X X   X   X     X           X X   X X X   X   X     X X   X   X     X X   X X   X \n"
		"X         X       X     X   X   X     X X X X   X   X X     X   X   X       X         X X   X X   X     \n"
		"X     X X   X   X X         X X X X     X   X     X X X         X X X X         X X X     X X   X X   X \n"
		"X X   X X   X X   X X X X X   X   X     X   X       X X X X X X   X   X X X X X   X   X     X X X   X   \n"
		"X X X       X X     X     X   X   X     X X X X   X X   X     X   X     X     X     X X     X   X     X \n"
		"X X X   X X   X                   X X X   X   X     X             X       X       X   X   X X X X X X   \n"
		"X   X     X   X     X X X     X X     X   X   X   X X   X X X X   X     X             X     X X X   X X \n"
		"X   X         X       X   X   X               X     X     X       X X X X X X   X   X   X X X   X X X   \n"
		"X X X     X X   X   X     X X   X X X     X X   X X X   X     X X   X X X             X   X       X X X \n"
		"X X   X   X X   X X   X X X       X   X   X X   X   X X   X   X X   X   X     X X X X X X     X   X     \n"
		"X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   \n"
		"X     X X       X X X   X   X   X     X X       X X X     X X     X   X X X X X X   X X   X X       X X \n"
		"X X X X X   X X     X   X   X X   X X X X   X X     X X X X     X   X   X X           X       X X   X   \n"
		"X   X   X     X             X X X   X   X     X X X X   X   X   X       X X           X X         X X X \n"
		"X   X   X     X X X X   X X     X   X   X       X   X   X X X               X     X X X X   X   X   X   \n"
		"X   X X X X     X   X     X         X X X X       X X         X     X   X     X X     X   X X   X     X \n"
		"X X   X   X     X   X     X X X X X   X   X X X     X X   X   X         X X X   X X     X X     X   X   \n"
		"X X   X   X     X X X X     X     X   X     X     X X X X X         X X   X   X     X X   X X X X     X \n"
		"X         X X X   X   X               X     X X     X   X       X X X     X       X   X X   X     X     \n"
		"X     X X     X   X   X     X X X X   X     X   X X X   X X     X   X       X X   X   X   X       X X X \n"
		"X X   X               X       X       X X   X X X   X   X       X       X X X   X   X X     X X   X X   \n"
		"X X X   X X X     X X   X   X     X X           X X X X     X X X X X   X     X X X   X     X         X \n"
		"X X       X   X   X X   X X   X   X     X X X       X X   X X       X X X   X X     X   X     X   X X   \n"
		"X   X   X     X X       X     X X     X X     X   X X X   X   X X   X           X X       X X X X   X X \n"
		"X   X X   X X X X   X X   X X X X   X X   X X X     X     X X X           X       X X X X X X       X   \n"
		"X   X X X   X   X     X X   X   X X   X         X X X           X X     X X X X     X X   X X         X \n"
		"X X     X   X   X       X     X X       X X     X   X X   X     X X         X   X X X X X X   X     X   \n"
		"X X         X X X X             X X X     X X X X X X         X X X   X X X X X X X X X X X       X   X \n"
		"X X X X X X   X   X X X X X   X X     X X X   X     X       X     X X       X X X       X X   X     X   \n"
		"X   X     X   X     X   X     X X X X     X X     X X     X X X   X   X X X X X X   X   X     X   X   X \n"
		"X             X       X     X   X   X               X   X X   X X   X     X X           X X   X X X     \n"
		"X   X X X X   X     X X X X   X     X     X   X X X X   X   X X X     X X   X             X X X   X   X \n"
		"X     X       X           X     X X     X     X X   X     X X X       X   X       X   X   X     X       \n"
		"X   X X   X X     X   X   X X   X   X X       X   X X         X X X X   X           X   X           X X \n"
		"X X   X X X X X       X     X         X X X X X     X     X   X X         X X X     X     X   X X   X   \n"
		"X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X \n";
	DoTest(text, SymbolShape::SQUARE, expected);
}
