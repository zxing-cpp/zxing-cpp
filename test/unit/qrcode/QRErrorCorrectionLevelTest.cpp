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
#include "qrcode/QRErrorCorrectionLevel.h"

using namespace ZXing;
using namespace ZXing::QRCode;

TEST(QRErrorCorrectionLevelTest, ForBits)
{
    EXPECT_EQ(ErrorCorrectionLevel::Medium, ECLevelFromBits(0));
	EXPECT_EQ(ErrorCorrectionLevel::Low, ECLevelFromBits(1));
	EXPECT_EQ(ErrorCorrectionLevel::High, ECLevelFromBits(2));
	EXPECT_EQ(ErrorCorrectionLevel::Quality, ECLevelFromBits(3));
}
