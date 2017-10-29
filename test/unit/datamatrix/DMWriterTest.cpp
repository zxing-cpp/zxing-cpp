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
#include "BitMatrix.h"
#include "datamatrix/DMWriter.h"
#include "datamatrix/DMSymbolShape.h"

using namespace ZXing::DataMatrix;

TEST(DMWriterTest, ImageWriter)
{
    int bigEnough = 64;
	Writer writer;
	writer.setShapeHint(SymbolShape::SQUARE);
	auto matrix = writer.encode(L"Hello Google", bigEnough, bigEnough);
    EXPECT_LE(matrix.width(), bigEnough);
	EXPECT_LE(matrix.height(), bigEnough);
}

TEST(DMWriterTest, Writer2)
{
	int bigEnough = 14;
	Writer writer;
	writer.setShapeHint(SymbolShape::SQUARE);
	auto matrix = writer.encode(L"Hello Me", bigEnough, bigEnough);
	EXPECT_EQ(matrix.width(), bigEnough);
	EXPECT_EQ(matrix.height(), bigEnough);
}

TEST(DMWriterTest, TooSmallSize)
{
    // The DataMatrix will not fit in this size, so the matrix should come back bigger
    int tooSmall = 8;
	Writer writer;
	auto matrix = writer.encode(L"http://www.google.com/", tooSmall, tooSmall);
	EXPECT_GT(matrix.width(), tooSmall);
	EXPECT_GT(matrix.height(), tooSmall);
}
