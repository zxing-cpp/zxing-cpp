/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BitMatrixIO.h"
#include "pdf417/PDFWriter.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::Pdf417;

TEST(PDF417WriterTest, DataMatrixImageWriter)
{
	Writer writer;
	writer.setMargin(0);
	int size = 64;
	BitMatrix matrix = writer.encode(L"Hello Google", size, size);
	auto actual = ToString(matrix, 'X', ' ', true);
	EXPECT_EQ(actual,
        "X X X X X X X X   X   X   X       X X X X X   X   X   X X X X X     X X   X   X         X X           X X X X   X X     X     X X X     X X X X   X   X   X X X X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X   X   X X X X X     X X   X   X         X X           X X X X   X X     X     X X X     X X X X   X   X   X X X X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X   X   X X X X X     X X   X   X         X X           X X X X   X X     X     X X X     X X X X   X   X   X X X X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X   X   X X X X X     X X   X   X         X X           X X X X   X X     X     X X X     X X X X   X   X   X X X X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X X   X   X       X X X   X X X X   X       X X   X X       X   X X     X     X X X X X X     X X X X   X   X     X             X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X X   X   X       X X X   X X X X   X       X X   X X       X   X X     X     X X X X X X     X X X X   X   X     X             X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X X   X   X       X X X   X X X X   X       X X   X X       X   X X     X     X X X X X X     X X X X   X   X     X             X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X X   X   X       X X X   X X X X   X       X X   X X       X   X X     X     X X X X X X     X X X X   X   X     X             X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X   X X X X X X       X     X X X X X   X X   X         X   X X   X X           X X X X   X X X   X   X       X X X X X X   X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X   X X X X X X       X     X X X X X   X X   X         X   X X   X X           X X X X   X X X   X   X       X X X X X X   X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X   X X X X X X       X     X X X X X   X X   X         X   X X   X X           X X X X   X X X   X   X       X X X X X X   X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X   X X X X X X       X     X X X X X   X X   X         X   X X   X X           X X X X   X X X   X   X       X X X X X X   X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X   X   X X X X     X X X X X   X       X     X X X   X X X       X   X X         X X   X           X   X   X X X X     X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X   X   X X X X     X X X X X   X       X     X X X   X X X       X   X X         X X   X           X   X   X X X X     X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X   X   X X X X     X X X X X   X       X     X X X   X X X       X   X X         X X   X           X   X   X X X X     X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X   X   X X X X     X X X X X   X       X     X X X   X X X       X   X X         X X   X           X   X   X X X X     X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X X X         X X     X X X X X     X     X       X X   X X X X X   X       X X   X X X   X X X X   X   X X X     X X X     X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X X X         X X     X X X X X     X     X       X X   X X X X X   X       X X   X X X   X X X X   X   X X X     X X X     X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X X X         X X     X X X X X     X     X       X X   X X X X X   X       X X   X X X   X X X X   X   X X X     X X X     X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X   X X X         X X     X X X X X     X     X       X X   X X X X X   X       X X   X X X   X X X X   X   X X X     X X X     X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X   X   X X X X   X         X     X X X             X   X X   X X   X X X X X   X         X     X X X   X   X X X X X   X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X   X   X X X X   X         X     X X X             X   X X   X X   X X X X X   X         X     X X X   X   X X X X X   X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X   X   X X X X   X         X     X X X             X   X X   X X   X X X X X   X         X     X X X   X   X X X X X   X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X   X   X X X X   X         X     X X X             X   X X   X X   X X X X X   X         X     X X X   X   X X X X X   X         X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X     X X X   X X X X X   X X X X   X X   X X     X X X X   X X   X X X   X X       X         X X   X     X X X   X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X     X X X   X X X X X   X X X X   X X   X X     X X X X   X X   X X X   X X       X         X X   X     X X X   X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X     X X X   X X X X X   X X X X   X X   X X     X X X X   X X   X X X   X X       X         X X   X     X X X   X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X   X     X X X   X X X X X   X X X X   X X   X X     X X X X   X X   X X X   X X       X         X X   X     X X X   X X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X     X   X X         X       X   X X X X           X   X   X X X X X       X   X X X     X   X   X X X X X X   X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X     X   X X         X       X   X X X X           X   X   X X X X X       X   X X X     X   X   X X X X X X   X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X     X   X X         X       X   X X X X           X   X   X X X X X       X   X X X     X   X   X X X X X X   X X X       X X X X X X X   X       X   X     X \n"
        "X X X X X X X X   X   X   X       X X X X X   X     X   X X         X       X   X X X X           X   X   X X X X X       X   X X X     X   X   X X X X X X   X X X       X X X X X X X   X       X   X     X \n");
}
