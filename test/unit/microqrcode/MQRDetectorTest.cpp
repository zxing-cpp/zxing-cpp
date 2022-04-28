/*
 * Copyright 2022 KURZ Digital Solutions GmbH & Co. KG
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

#include "microqrcode/MQRDetector.h"

#include "BitMatrixIO.h"
#include "DecodeHints.h"

#include "gtest/gtest.h"

using namespace ZXing;
using namespace ZXing::MicroQRCode;

namespace {

BitMatrix LoadCode()
{
	return ParseBitMatrix("XXXXXXX X X X X\n"
						  "X     X    X X \n"
						  "X XXX X XXXXXXX\n"
						  "X XXX X X X  XX\n"
						  "X XXX X    X XX\n"
						  "X     X X X X X\n"
						  "XXXXXXX  X  XX \n"
						  "         X X  X\n"
						  "XXXXXX    X X X\n"
						  "   X  XX    XXX\n"
						  "XXX XX XXXX XXX\n"
						  " X    X  XXX X \n"
						  "X XXXXX XXX X X\n"
						  " X    X  X XXX \n"
						  "XXX XX X X XXXX\n",
						  88, false);
}

BitMatrix ScaleCode(BitMatrix&& bitMatrix, const int moduleSize, const int quietZone)
{
	// Inflate bit matrix since corner finder does not work with pure barcodes.
	return Inflate(std::move(bitMatrix), (bitMatrix.width() + 2 * quietZone) * moduleSize,
				   (bitMatrix.height() + 2 * quietZone) * moduleSize, quietZone * moduleSize);
}

} // namespace

TEST(MicroQRDetectorTest, DetectPureBarcodeNoQuietZone)
{
	const auto testCode = LoadCode();
	DecodeHints hints;
	Detector detector{testCode};
	auto result = detector.detect(hints);
	ASSERT_FALSE(result.isValid());

	hints.setIsPure(true);
	result = detector.detect(hints);
	ASSERT_EQ(testCode, result.bits());
}

TEST(MicroQRDetectorTest, DetectPureBarcodeQuietZone)
{
	const auto testCode = ScaleCode(LoadCode(), 1, 2);
	DecodeHints hints;
	Detector detector{testCode};
	auto result = detector.detect(hints);
	ASSERT_FALSE(result.isValid());

	hints.setIsPure(true);
	result = detector.detect(hints);
	ASSERT_EQ(LoadCode(), result.bits());
}

TEST(MicroQRDetectorTest, DetectPureBarcodeQuietZoneAndModuleSize2)
{
	const auto testCode = ScaleCode(LoadCode(), 2, 2);
	DecodeHints hints;
	Detector detector{testCode};
	auto result = detector.detect(hints);
	ASSERT_EQ(LoadCode(), result.bits());

	hints.setIsPure(true);
	result = detector.detect(hints);
	ASSERT_EQ(LoadCode(), result.bits());
}

TEST(MicroQRDetectorTest, DetectScaledPureBarcodeQuietZone)
{
	auto testCode = ScaleCode(LoadCode(), 12, 2);

	DecodeHints hints;
	Detector detector{testCode};
	auto result = detector.detect(hints);
	ASSERT_EQ(LoadCode(), result.bits());

	hints.setIsPure(true);
	result = detector.detect(hints);
	ASSERT_EQ(LoadCode(), result.bits());
}

TEST(MicroQRDetectorTest, DetectRotatedBarcode)
{
	auto testCode = ScaleCode(LoadCode(), 12, 2);

	for (auto loop : {0, 90, 180, 270}) {
		DecodeHints hints;
		Detector detector{testCode};
		auto result = detector.detect(hints);
		ASSERT_EQ(LoadCode(), result.bits()) << "Rotation " << loop;
		testCode.rotate90();
	}
}

TEST(MicroQRDetectorTest, DetectNoBarcode)
{
	auto testCode = ScaleCode(BitMatrix{15}, 12, 2);

	DecodeHints hints;
	Detector detector{testCode};
	auto result = detector.detect(hints);
	ASSERT_FALSE(result.isValid());

	hints.setIsPure(true);
	result = detector.detect(hints);
	ASSERT_FALSE(result.isValid());
}
