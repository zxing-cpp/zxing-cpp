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
#include "qrcode/QRWriter.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "BitMatrix.h"
#include "BitMatrixUtility.h"

using namespace ZXing;
using namespace ZXing::QRCode;

namespace {

	void DoTest(const std::wstring& contents, ErrorCorrectionLevel ecLevel, int resolution, const char* expected) {
		Writer writer;
		writer.setErrorCorrectionLevel(ecLevel);
		auto matrix = writer.encode(contents, resolution, resolution);
		auto actual = Utility::ToString(matrix, 'X', ' ', true);
		EXPECT_EQ(matrix.width(), resolution);
		EXPECT_EQ(matrix.height(), resolution);
		EXPECT_EQ(actual, expected);
	}

}

TEST(QRWriterTest, OverSize)
{
	// The QR should be multiplied up to fit, with extra padding if necessary
	int bigEnough = 256;
	Writer writer;
	BitMatrix matrix = writer.encode(L"http://www.google.com/", bigEnough, bigEnough);
	EXPECT_EQ(matrix.width(), bigEnough);
	EXPECT_EQ(matrix.height(), bigEnough);

	// The QR will not fit in this size, so the matrix should come back bigger
	int tooSmall = 20;
	matrix = writer.encode(L"http://www.google.com/", tooSmall, tooSmall);
	EXPECT_GT(matrix.width(), tooSmall);
	EXPECT_GT(matrix.height(), tooSmall);

	// We should also be able to handle non-square requests by padding them
	int strangeWidth = 500;
	int strangeHeight = 100;
	matrix = writer.encode(L"http://www.google.com/", strangeWidth, strangeHeight);
	EXPECT_EQ(matrix.width(), strangeWidth);
	EXPECT_EQ(matrix.height(), strangeHeight);
}

TEST(QRWriterTest, RegressionTest)
{
	DoTest(L"http://www.google.com/", ErrorCorrectionLevel::Medium, 99,
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X                   X X X       X X X X X X             X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X                   X X X       X X X X X X             X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X                   X X X       X X X X X X             X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                        X X X                               X X X             X X X X X X       X X X                   X X X       X X X                               X X X                         \n"
		"                        X X X                               X X X             X X X X X X       X X X                   X X X       X X X                               X X X                         \n"
		"                        X X X                               X X X             X X X X X X       X X X                   X X X       X X X                               X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X                   X X X X X X X X X             X X X             X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X                   X X X X X X X X X             X X X             X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X                   X X X X X X X X X             X X X             X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X       X X X       X X X X X X X X X       X X X                         \n"
		"                        X X X                               X X X       X X X       X X X X X X X X X       X X X                   X X X                               X X X                         \n"
		"                        X X X                               X X X       X X X       X X X X X X X X X       X X X                   X X X                               X X X                         \n"
		"                        X X X                               X X X       X X X       X X X X X X X X X       X X X                   X X X                               X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X       X X X       X X X       X X X       X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X       X X X       X X X       X X X       X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X       X X X       X X X       X X X       X X X X X X X X X X X X X X X X X X X X X                         \n"
		"                                                                        X X X       X X X                   X X X X X X                                                                               \n"
		"                                                                        X X X       X X X                   X X X X X X                                                                               \n"
		"                                                                        X X X       X X X                   X X X X X X                                                                               \n"
		"                        X X X                   X X X       X X X X X X X X X X X X       X X X                   X X X       X X X X X X X X X X X X X X X             X X X                         \n"
		"                        X X X                   X X X       X X X X X X X X X X X X       X X X                   X X X       X X X X X X X X X X X X X X X             X X X                         \n"
		"                        X X X                   X X X       X X X X X X X X X X X X       X X X                   X X X       X X X X X X X X X X X X X X X             X X X                         \n"
		"                              X X X X X X X X X       X X X             X X X             X X X X X X       X X X X X X X X X X X X             X X X X X X       X X X                               \n"
		"                              X X X X X X X X X       X X X             X X X             X X X X X X       X X X X X X X X X X X X             X X X X X X       X X X                               \n"
		"                              X X X X X X X X X       X X X             X X X             X X X X X X       X X X X X X X X X X X X             X X X X X X       X X X                               \n"
		"                                    X X X                   X X X       X X X       X X X       X X X X X X X X X X X X                   X X X X X X X X X X X X                                     \n"
		"                                    X X X                   X X X       X X X       X X X       X X X X X X X X X X X X                   X X X X X X X X X X X X                                     \n"
		"                                    X X X                   X X X       X X X       X X X       X X X X X X X X X X X X                   X X X X X X X X X X X X                                     \n"
		"                              X X X X X X       X X X X X X                         X X X       X X X       X X X                   X X X X X X             X X X X X X                               \n"
		"                              X X X X X X       X X X X X X                         X X X       X X X       X X X                   X X X X X X             X X X X X X                               \n"
		"                              X X X X X X       X X X X X X                         X X X       X X X       X X X                   X X X X X X             X X X X X X                               \n"
		"                        X X X       X X X       X X X X X X X X X                   X X X                                           X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X       X X X X X X X X X                   X X X                                           X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X       X X X X X X X X X                   X X X                                           X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X                         X X X X X X X X X X X X       X X X       X X X X X X X X X                   X X X             X X X                               \n"
		"                        X X X       X X X                         X X X X X X X X X X X X       X X X       X X X X X X X X X                   X X X             X X X                               \n"
		"                        X X X       X X X                         X X X X X X X X X X X X       X X X       X X X X X X X X X                   X X X             X X X                               \n"
		"                                    X X X                   X X X X X X X X X       X X X X X X X X X       X X X X X X             X X X X X X X X X X X X X X X                                     \n"
		"                                    X X X                   X X X X X X X X X       X X X X X X X X X       X X X X X X             X X X X X X X X X X X X X X X                                     \n"
		"                                    X X X                   X X X X X X X X X       X X X X X X X X X       X X X X X X             X X X X X X X X X X X X X X X                                     \n"
		"                                                X X X             X X X X X X             X X X             X X X             X X X X X X X X X       X X X X X X X X X                               \n"
		"                                                X X X             X X X X X X             X X X             X X X             X X X X X X X X X       X X X X X X X X X                               \n"
		"                                                X X X             X X X X X X             X X X             X X X             X X X X X X X X X       X X X X X X X X X                               \n"
		"                        X X X X X X       X X X             X X X                               X X X       X X X X X X X X X X X X X X X X X X X X X X X X X X X                                     \n"
		"                        X X X X X X       X X X             X X X                               X X X       X X X X X X X X X X X X X X X X X X X X X X X X X X X                                     \n"
		"                        X X X X X X       X X X             X X X                               X X X       X X X X X X X X X X X X X X X X X X X X X X X X X X X                                     \n"
		"                                                                        X X X X X X X X X                   X X X       X X X                   X X X       X X X                                     \n"
		"                                                                        X X X X X X X X X                   X X X       X X X                   X X X       X X X                                     \n"
		"                                                                        X X X X X X X X X                   X X X       X X X                   X X X       X X X                                     \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X             X X X X X X       X X X       X X X       X X X X X X                                           \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X             X X X X X X       X X X       X X X       X X X X X X                                           \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X       X X X             X X X X X X       X X X       X X X       X X X X X X                                           \n"
		"                        X X X                               X X X             X X X X X X       X X X       X X X       X X X                   X X X X X X X X X X X X X X X                         \n"
		"                        X X X                               X X X             X X X X X X       X X X       X X X       X X X                   X X X X X X X X X X X X X X X                         \n"
		"                        X X X                               X X X             X X X X X X       X X X       X X X       X X X                   X X X X X X X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X       X X X X X X X X X X X X X X X       X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X       X X X X X X X X X X X X X X X       X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X       X X X             X X X       X X X X X X       X X X X X X X X X X X X X X X       X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X             X X X             X X X       X X X X X X X X X X X X X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X             X X X             X X X       X X X X X X X X X X X X X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X             X X X             X X X       X X X X X X X X X X X X X X X             X X X X X X X X X X X X                         \n"
		"                        X X X       X X X X X X X X X       X X X             X X X       X X X X X X X X X X X X             X X X X X X X X X                   X X X                               \n"
		"                        X X X       X X X X X X X X X       X X X             X X X       X X X X X X X X X X X X             X X X X X X X X X                   X X X                               \n"
		"                        X X X       X X X X X X X X X       X X X             X X X       X X X X X X X X X X X X             X X X X X X X X X                   X X X                               \n"
		"                        X X X                               X X X                   X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X                               \n"
		"                        X X X                               X X X                   X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X                               \n"
		"                        X X X                               X X X                   X X X X X X       X X X       X X X                   X X X X X X X X X X X X X X X                               \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X X X X X X X       X X X X X X       X X X       X X X                         X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X X X X X X X       X X X X X X       X X X       X X X                         X X X X X X X X X                         \n"
		"                        X X X X X X X X X X X X X X X X X X X X X       X X X X X X X X X       X X X X X X       X X X       X X X                         X X X X X X X X X                         \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
		"                                                                                                                                                                                                      \n"
	);
}
