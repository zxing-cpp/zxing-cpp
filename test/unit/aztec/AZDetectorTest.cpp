/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2013 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "aztec/AZDetector.h"

#include "BitMatrixIO.h"
#include "DecoderResult.h"
#include "PseudoRandom.h"
#include "Utf.h"
#include "aztec/AZDecoder.h"
#include "aztec/AZDetectorResult.h"

#include "gtest/gtest.h"
#include <string_view>
#include <vector>

using namespace ZXing;

namespace {

	struct Point {
		int x;
		int y;
	};

	std::vector<Point> GetOrientationPoints(const BitMatrix& matrix, bool isCompact) {
		int center = matrix.width() / 2;
		int offset = isCompact ? 5 : 7;
		std::vector<Point> result;
		result.reserve(12);
		for (int xSign : { -1, 1}) {
			for (int ySign : { -1, 1}) {
				result.push_back({ center + xSign * offset, center + ySign * offset });
				result.push_back({ center + xSign * (offset - 1), center + ySign * offset });
				result.push_back({ center + xSign * offset, center + ySign * (offset - 1) });
			}
		}
		return result;
	}

	// Test that we can tolerate errors in the parameter locator bits
	void TestErrorInOrientationBits(std::string_view data, int nbLayers, bool isCompact, const BitMatrix &matrix_)
	{
		PseudoRandom random(std::hash<std::string_view>()(data));
		auto orientationPoints = GetOrientationPoints(matrix_, isCompact);
		for (bool mirror : { false, true }) {
			BitMatrix matrix = matrix_.copy();
			if (mirror)
				matrix.mirror();
			for (int i = 0; i < 4; ++i) {
				Aztec::DetectorResult r = Aztec::Detect(matrix, true, false);
				EXPECT_EQ(r.isValid(), true);
				EXPECT_EQ(r.nbLayers(), nbLayers);
				EXPECT_EQ(r.isCompact(), isCompact);
				EXPECT_EQ(r.isMirrored(), mirror);
				EXPECT_EQ(data, ToUtf8(Aztec::Decode(r).text()));

				// Systematically try every possible 1- and 2-bit error.
				for (int error1 = 0; error1 < Size(orientationPoints); error1++) {
					for (int error2 = error1; error2 < Size(orientationPoints); error2++) {
						BitMatrix copy = matrix.copy();
						copy.flip(orientationPoints[error1].x, orientationPoints[error1].y);
						if (error2 > error1 && (nbLayers > 1 || !mirror)) { // in ErrorInModeMessageZero and mirror==true test only 1-bit errors
							// if error2 == error1, we only test a single error
							copy.flip(orientationPoints[error2].x, orientationPoints[error2].y);
						}
						Aztec::DetectorResult r = Aztec::Detect(copy, true, false);
						EXPECT_EQ(r.isValid(), true);
						EXPECT_EQ(r.nbLayers(), nbLayers);
						EXPECT_EQ(r.isCompact(), isCompact);
						EXPECT_EQ(r.isMirrored(), mirror);
						EXPECT_EQ(data, ToUtf8(Aztec::Decode(r).text()));
					}
				}

				// here used to be a test for 3-bit errors but since we determine both the rotation and the mirrored info
				// from the orentation bits (as suggested in the specification) this does not work anymore.

				matrix.rotate90();
			}
		}
	}
} // anonymous

TEST(AZDetectorTest, ErrorInModeMessageZero)
{
	// Layers=1, CodeWords=1. So the ModeMessage and its Reed-Solomon bits will be completely zero!
	TestErrorInOrientationBits("X", 1, true, ParseBitMatrix(
		"    X X X X X X X   X X X X X \n"
		"X X X X   X     X X         X \n"
		"    X X                 X   X \n"
		"X X X X X X X X X X X X X   X \n"
		"X X   X               X     X \n"
		"X X   X   X X X X X   X     X \n"
		"X X   X   X       X   X   X X \n"
		"      X   X   X   X   X     X \n"
		"X X   X   X       X   X   X X \n"
		"      X   X X X X X   X     X \n"
		"X     X               X     X \n"
		"  X   X X X X X X X X X X X   \n"
		"  X                         X \n"
		"X     X X X X   X     X       \n"
		"X   X     X X X X       X     \n"
		, 'X', true)
	);
}

TEST(AZDetectorTest, ErrorInOrientationBitsCompact)
{
	TestErrorInOrientationBits("This is an example Aztec symbol for Wikipedia.", 3, true, ParseBitMatrix(
		"X     X X       X     X X     X     X         \n"
		"X         X     X X     X   X X   X X       X \n"
		"X X   X X X X X   X X X                 X     \n"
		"X X                 X X   X       X X X X X X \n"
		"    X X X   X   X     X X X X         X X     \n"
		"  X X X   X X X X   X     X   X     X X   X   \n"
		"        X X X X X     X X X X   X   X     X   \n"
		"X       X   X X X X X X X X X X X     X   X X \n"
		"X   X     X X X               X X X X   X X   \n"
		"X     X X   X X   X X X X X   X X   X   X X X \n"
		"X   X         X   X       X   X X X X       X \n"
		"X       X     X   X   X   X   X   X X   X     \n"
		"      X   X X X   X       X   X     X X X     \n"
		"    X X X X X X   X X X X X   X X X X X X   X \n"
		"  X X   X   X X               X X X   X X X X \n"
		"  X   X       X X X X X X X X X X X X   X X   \n"
		"  X X   X       X X X   X X X       X X       \n"
		"  X               X   X X     X     X X X     \n"
		"  X   X X X   X X   X   X X X X   X   X X X X \n"
		"    X   X   X X X   X   X   X X X X     X     \n"
		"        X               X                 X   \n"
		"        X X     X   X X   X   X   X       X X \n"
		"  X   X   X X       X   X         X X X     X \n"
		, 'X', true)
	);
}

TEST(AZDetectorTest, ErrorInOrientationBitsNotCompact)
{
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyz";
	TestErrorInOrientationBits(alphabet + alphabet + alphabet, 6, false, ParseBitMatrix(
		"    X   X     X     X     X   X X X X   X   X   X     X X     X X       X X X X   \n"
		"  X         X   X         X X X X X   X   X X X   X   X X X X X   X X X       X   \n"
		"    X   X       X X X X X   X X X X   X X   X X X X X   X X X     X   X X X   X   \n"
		"      X     X     X   X X X X     X   X       X X     X X       X X X         X   \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"X X X               X X X       X           X X X   X     X   X   X X     X X   X \n"
		"        X X X X X X     X   X X   X   X X     X X   X X X X     X X     X     X   \n"
		"X   X X       X   X X X X     X X X X     X X X X   X X X X X       X       X     \n"
		"    X   X X   X X       X     X     X   X   X     X X   X     X X   X   X     X   \n"
		"  X X           X X   X   X       X X       X X X X     X     X X   X             \n"
		"  X     X   X   X X X     X X         X X   X X X X     X X X X X     X X X X   X \n"
		"      X     X X X X X X X X X X   X       X   X X   X     X   X           X X X X \n"
		"X X     X     X X     X   X   X     X   X X X X X X       X X   X       X X   X X \n"
		"    X     X X       X X X X X     X   X           X   X         X   X       X     \n"
		"  X X   X       X         X X X X X X X X X X X X X X X X     X     X X X X X X X \n"
		"X X X       X X   X X X X   X                       X X X   X     X X       X X   \n"
		"  X   X X X X   X   X X   X X   X X X X X X X X X   X         X   X     X   X X   \n"
		"      X     X X X           X   X               X   X     X       X X X   X   X X \n"
		"    X   X       X X     X   X   X   X X X X X   X   X   X X X X   X     X         \n"
		"X   X X         X X X X   X X   X   X       X   X   X X X X   X X X X     X X   X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"  X       X   X   X X   X   X   X   X       X   X   X X   X X   X X X       X X   \n"
		"  X   X X   X X X X     X X X   X   X X X X X   X   X   X   X   X X     X X   X X \n"
		"  X X       X X X         X X   X               X   X X     X   X X   X   X     X \n"
		"    X   X   X   X X X     X X   X X X X X X X X X   X   X X X X X X     X   X     \n"
		"X   X X           X     X   X                       X   X X   X   X X X     X X   \n"
		"X X X   X X   X     X   X   X X X X X X X X X X X X X X   X   X X X     X   X X   \n"
		"  X   X   X X X               X   X   X     X     X     X   X   X             X   \n"
		"X   X X X   X X     X X       X   X X X X   X X X X X   X X X X X   X   X X     X \n"
		"    X X   X         X X X     X           X       X X   X         X               \n"
		"X X     X     X X     X X     X         X     X X X       X   X X       X   X     \n"
		"  X       X X   X X X     X     X X       X X   X X X     X X       X X     X X   \n"
		"  X X   X   X X X X X       X X       X X X   X X X X   X X X   X X X   X X X X X \n"
		"X X         X X X X   X   X         X X   X X   X     X           X X         X   \n"
		"    X X X X   X X     X   X   X X   X   X   X X X   X X X X X   X   X X X   X     \n"
		"X X       X   X X X         X       X X   X       X X     X X     X X     X   X X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"X X       X X X       X X     X X     X     X     X           X   X         X     \n"
		"X   X X X   X     X X X   X X X X X   X X   X X X X X     X     X       X   X X   \n"
		"  X   X     X X   X     X X X   X X X X   X   X   X X X X X     X     X       X   \n"
		"        X X       X X X       X X     X X X     X   X     X           X X   X     \n"
		, 'X', true)
	);
}

TEST(AZDetectorTest, ReaderInitFull2Layers)
{
	{
		// Null (not set)
		auto r = Aztec::Detect(ParseBitMatrix(
			"      X X X   X   X X   X X X X   X X X X X   \n"
			"    X   X X X X   X X X     X X X         X   \n"
			"  X X   X X   X   X X   X X X   X X X     X X \n"
			"  X X X   X X X   X X X X   X   X   X   X   X \n"
			"    X X X X         X               X X X   X \n"
			"X       X X X X X X X X X X X X X X X     X X \n"
			"X     X X X                       X     X   X \n"
			"X   X X X X   X X X X X X X X X   X           \n"
			"X X X   X X   X               X   X       X X \n"
			"X   X X   X   X   X X X X X   X   X   X X   X \n"
			"X   X     X   X   X       X   X   X X X   X X \n"
			"  X   X   X   X   X   X   X   X   X   X   X   \n"
			"  X X X X X   X   X       X   X   X         X \n"
			"X   X   X X   X   X X X X X   X   X X     X   \n"
			"X       X X   X               X   X X X X   X \n"
			"  X   X   X   X X X X X X X X X   X   X X   X \n"
			"X     X X X                       X X         \n"
			"X X   X   X X X X X X X X X X X X X X X X   X \n"
			"X X         X   X       X   X X       X X     \n"
			"    X     X         X X X X   X X X     X X   \n"
			"    X     X X X         X   X X X     X       \n"
			"    X X X         X   X   X X   X       X   X \n"
			"  X   X X X             X   X   X       X     \n"
		), false /*isPure*/, false /*tryHarder*/);
		EXPECT_TRUE(r.isValid());
		EXPECT_FALSE(r.readerInit());
		EXPECT_FALSE(r.isCompact());
		EXPECT_EQ(r.nbLayers(), 2);
	}
	{
		// Set
		auto r = Aztec::Detect(ParseBitMatrix(
			"      X X X   X   X X   X X X X   X X X X X   \n"
			"    X   X X X X   X X X     X X X         X   \n"
			"  X X   X X   X   X X   X X X   X X X     X X \n"
			"  X X X   X X X   X X X X   X   X   X   X   X \n"
			"    X X X X         X   X           X X X   X \n"
			"X       X X X X X X X X X X X X X X X     X X \n"
			"X     X X X                       X     X   X \n"
			"X   X X   X   X X X X X X X X X   X           \n"
			"X X X     X   X               X   X       X X \n"
			"X   X X   X   X   X X X X X   X   X   X X   X \n"
			"X   X   X X   X   X       X   X   X X X   X X \n"
			"  X   X   X   X   X   X   X   X   X   X   X   \n"
			"  X X X X X   X   X       X   X   X         X \n"
			"X   X     X   X   X X X X X   X   X       X   \n"
			"X         X   X               X   X X X X   X \n"
			"  X   X X X   X X X X X X X X X   X   X X   X \n"
			"X     X   X                       X X         \n"
			"X X   X   X X X X X X X X X X X X X X X X   X \n"
			"X X           X X X X         X X     X X     \n"
			"    X     X         X X X X   X X X     X X   \n"
			"    X     X X X         X   X X X     X       \n"
			"    X X X         X   X   X X   X       X   X \n"
			"  X   X X X             X   X   X       X     \n"
		), true /*isPure*/, false /*tryHarder*/);
		EXPECT_TRUE(r.isValid());
		EXPECT_TRUE(r.readerInit());
		EXPECT_FALSE(r.isCompact());
		EXPECT_EQ(r.nbLayers(), 2);
	}
}

TEST(AZDetectorTest, ReaderInitFull22Layers)
{
	// Set
	auto r = Aztec::Detect(ParseBitMatrix(
		"            X X   X   X X X     X       X   X     X         X   X     X X   X   X       X X   X X X         X       X   X     X   X         X X X X X X   X   X   X   X X   X X     X     X X X X     X   X X X X   X X   \n"
		"        X X     X X   X   X X X X   X X   X           X     X   X   X X   X   X X X X X X X       X   X         X X X   X X     X X   X             X         X   X X   X X   X   X   X   X X     X X X X     X     X   X \n"
		"    X   X   X   X       X           X X     X       X       X   X X       X X X     X   X   X X     X X   X X X X         X X X   X     X X X   X   X X     X       X X X   X   X     X X   X X X           X X   X X     \n"
		"      X X       X X     X X X     X       X     X   X   X       X     X       X             X   X     X X         X   X   X X   X       X     X X   X           X X X X   X   X X     X     X     X X         X   X   X   \n"
		"        X   X       X X   X       X X X X   X X X X X X X X X   X X X   X X X   X   X X X X X X X   X X X X X     X       X   X     X       X X X     X         X X X   X X X X X           X   X X X X X X X X       X   \n"
		"    X     X   X     X   X X         X X X X     X     X X X   X X     X       X X   X X     X X X X X X X X   X   X   X     X     X X   X           X           X     X         X   X   X X   X     X X X X   X X         \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"          X     X X     X X X X       X   X     X       X X           X X X               X   X     X X X X   X X X   X         X X X         X X   X X X   X   X X                     X         X X X   X         X X X \n"
		"X   X X X X X         X X X   X   X X       X X   X X X   X X X             X   X X       X   X       X     X X       X   X   X X   X X X X X   X   X   X       X X         X X     X X X     X X X X       X X       X   \n"
		"  X X         X X X     X X X X   X X X X X   X         X       X   X X   X     X X X X X X   X X   X X   X   X       X   X   X X X     X X   X     X   X X   X X X X X   X     X     X X     X X     X X X       X   X   \n"
		"  X   X     X X           X X   X X         X   X   X X X   X   X X X X X X X X X       X   X   X X X X     X     X   X X X X X             X X X X X X     X     X     X   X   X     X           X     X X X   X X X   X \n"
		"  X X X X X   X X     X   X     X X X X   X         X X X X X X               X X X   X     X X X X     X X   X X           X X   X X X   X   X X   X X           X X X X X       X     X         X X X X X   X     X X X \n"
		"  X     X X X X       X X     X       X   X X X X X X X X   X X X   X X X X X     X X   X     X   X   X X   X X             X X X X   X X   X X   X X   X     X     X       X X         X X X X X X X       X X       X   \n"
		"X X X             X X   X X   X X X   X   X     X   X   X X X X   X   X X         X         X       X     X     X       X X         X         X   X X   X     X X       X     X   X X X X   X X     X X X X   X   X X X   \n"
		"X X   X   X X     X   X X X     X X   X     X   X X X   X X X     X         X X     X X X X X X X X         X X     X X X X       X X X     X X X X X X   X X     X     X X X X         X X X   X   X   X X X       X   X \n"
		"  X X X   X     X X   X X         X X X X X   X X       X   X   X X       X       X X X     X X     X X X           X X   X X X X             X X X     X X   X X X       X   X X     X             X X   X     X X   X X \n"
		"X X     X   X     X     X X X X X X     X X X     X X X   X   X X X     X X X   X X X X X   X       X X X   X X     X X X   X   X X   X X   X   X X X     X   X X X       X X X     X           X   X X     X     X X     \n"
		"      X         X     X X   X         X X X         X   X             X   X   X X   X       X   X   X   X                 X   X   X X   X     X X       X X X X X       X       X X         X   X X             X   X X   \n"
		"  X X   X   X   X X     X       X   X X X X X   X X   X       X X           X X   X X   X X     X X X X     X X   X     X X     X   X X     X X   X X   X   X X X X     X X X X X       X X X     X     X X X     X       \n"
		"X X X X       X X X   X       X     X X   X     X X           X   X X X   X         X         X       X       X     X X     X X     X   X X     X X       X   X X       X     X         X   X     X X X X X     X       X \n"
		"    X   X   X X X X X     X X X X   X   X   X X       X   X   X         X   X X   X     X X       X X       X   X     X           X   X     X     X X   X       X   X X     X X X X   X X X       X X X     X X           \n"
		"  X   X       X X   X X X       X X X   X     X X   X       X             X       X     X X X X X   X X       X X         X X   X X   X X             X     X   X X   X X     X   X     X       X X X X X     X   X X   X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"      X X     X X   X X X X X     X X     X   X     X X X X     X   X X       X   X X X       X X     X X X     X   X X         X     X X     X       X       X X X   X         X   X   X X     X   X   X X   X   X   X X \n"
		"          X X     X X       X X     X     X X       X     X X X X X   X     X         X   X X X   X         X   X   X     X X   X X X       X   X X   X       X X   X   X X X X X   X X   X X X   X     X   X   X X X     \n"
		"X   X X X           X X X X   X     X   X X   X   X           X X X   X X     X X   X         X X         X   X         X   X X X     X X     X   X   X X       X X X X       X             X           X             X   \n"
		"X X   X     X   X   X X X X X     X X     X X   X X X   X   X   X   X       X     X   X X X X X X       X   X X         X   X     X X       X   X X   X         X   X     X X X X     X   X X           X X X         X X \n"
		"  X       X           X X X X X X   X X X     X   X   X   X X   X     X   X         X X   X   X                 X X   X X   X X     X         X X X     X X     X     X   X   X   X X X     X   X X X X                 X \n"
		"  X     X X X     X X X     X     X   X   X X X X X   X   X           X   X X   X       X X   X   X   X X X X X   X X       X X X   X   X   X X   X X X X X X X X   X X X   X X               X   X     X   X       X X   \n"
		"  X     X     X X     X X X           X X X   X X   X X   X X X   X   X       X X X X     X         X X   X   X           X X X X X   X   X       X X X       X X         X   X X X X     X       X X   X X   X   X X X X \n"
		"    X X   X X   X               X X   X X X X X X X           X   X   X X   X X     X     X X         X   X X X X X   X X     X   X   X X   X X X   X X X X   X     X X X   X X     X   X   X   X X     X   X   X   X X X \n"
		"X   X X   X   X         X     X X   X X         X X     X         X   X   X     X     X X X   X   X     X     X         X   X X X     X   X   X X     X X X X X X   X         X X       X   X   X     X   X     X     X X \n"
		"  X         X X   X     X   X     X X   X   X   X   X X   X     X X X X   X X     X   X X X X     X X       X   X   X X X X X X       X   X X     X   X     X X X X X X   X X X X       X   X X   X   X   X X X     X   X \n"
		"X                 X   X   X X   X       X X               X X   X X   X   X   X   X X   X X X   X     X   X   X X     X X     X   X     X X   X X     X   X X X X     X X     X X     X X       X X X         X   X X     \n"
		"X         X X X X   X X X X     X   X   X   X   X       X   X X         X X X X         X   X X X X X       X X X X X X X     X X     X   X X     X         X   X X   X X X X X X X X X   X X X   X     X   X   X X X X   \n"
		"  X   X   X       X     X X X X   X X X X     X   X X X X X   X   X X X   X     X   X   X     X         X X   X X X   X X         X X X X X     X   X       X     X       X   X X         X   X X X     X         X X   X \n"
		"X   X     X X     X       X   X   X         X X X X   X X     X   X   X   X X           X         X   X     X               X X     X       X       X X X X   X X X X X     X X   X   X     X     X X X     X     X X   X \n"
		"  X X   X     X                 X             X X X     X       X   X X X X   X   X     X X X   X X X X   X     X   X   X   X X X   X X X     X X X X   X X       X     X       X             X   X X X X X     X X X     \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"      X   X     X X   X X       X X   X   X             X   X X X     X   X     X X   X   X   X   X     X X             X X X       X X   X   X   X     X     X X X   X X           X   X X   X     X   X X     X X X   X \n"
		"X X X   X   X X     X     X   X X       X   X X     X     X     X           X X         X   X   X X X     X X   X   X X X     X X   X       X X X X   X     X X             X X   X X X     X     X X X     X X   X     X \n"
		"X     X   X       X X               X X   X           X   X       X     X X   X           X   X   X   X       X             X   X X   X         X X X X X     X X X X                     X   X X X X X       X X X X     \n"
		"    X   X X X     X X X X X X X X X X X X X X X X   X   X     X X           X     X   X X   X X   X         X     X   X X X     X   X X     X X X   X   X X   X   X         X   X X X   X X X X X X     X   X X     X X   \n"
		"  X X   X     X   X X       X X               X           X X   X X X X X     X         X     X X   X X X X         X     X     X     X   X   X   X X   X   X   X     X   X     X   X X X X X     X     X X   X X X X X X \n"
		"X X X   X X X X   X X X   X X X X X     X X X       X           X           X X             X X X   X X   X X X X   X X X X X   X X     X X X X X X     X                   X X     X   X X X   X X     X   X X X         \n"
		"X X X   X     X   X X     X     X       X     X           X       X X   X       X X   X X     X X     X X X   X X X   X X       X X X     X   X   X   X X   X X X     X X     X X X X   X   X X X   X X X X   X X   X   X \n"
		"  X   X X X X   X   X       X X   X         X       X     X   X             X X   X         X X         X X X   X         X X   X   X X X   X   X   X X       X X     X   X X X X X                   X   X X X X X     X \n"
		"X         X   X   X X X X X   X X   X   X X   X   X X   X X X X   X   X           X     X     X X X   X   X   X           X X X         X X   X X X X X     X X     X     X   X     X X       X   X     X X   X X X X   X \n"
		"      X X X X X X       X   X     X X X     X   X   X           X X X   X X X X X X X   X X   X X X X X X X X X X X X X X X X         X X X X       X X   X X       X       X       X   X X       X X     X X     X X X   \n"
		"X   X   X X   X     X X X X   X     X     X       X     X   X   X X                   X     X   X                       X       X X     X       X                 X X   X         X X         X       X X           X     \n"
		"            X X   X X X               X X X X   X       X X   X   X   X     X X X X   X X X     X   X X X X X X X X X   X     X X X   X     X   X   X   X X   X   X     X X X     X   X   X X X     X   X X X X X   X   X \n"
		"X X     X X         X   X     X X X X     X   X         X   X   X   X X X X   X X X X   X X   X X   X               X   X     X     X X X     X             X X   X X     X   X X   X   X X   X         X X   X X         \n"
		"X X   X X X X X       X X X X X     X X   X X   X X X   X       X         X X X X   X   X X     X   X   X X X X X   X   X         X X   X X X   X X   X X     X     X X X   X   X   X X         X X X   X X X X X X       \n"
		"X X   X X X   X     X   X     X X X X   X X       X   X X   X X   X           X   X X X     X   X   X   X       X   X   X     X   X X     X   X X   X       X   X   X     X     X   X X X   X   X X   X   X   X X X X   X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"X             X X X X   X   X             X           X X   X         X       X X X X   X X   X X   X   X       X   X   X X       X X   X     X X X   X X   X X X X   X X X       X     X     X         X X         X X   \n"
		"X     X     X   X     X X X     X X     X X X X   X X X   X X     X     X X X X   X X     X     X   X   X X X X X   X   X X   X X     X   X X   X     X           X X   X   X     X X     X         X       X     X   X   \n"
		"X X   X X           X   X X X X       X X X   X   X   X     X   X X     X     X   X   X X   X   X   X               X   X   X   X X     X X   X X X   X   X         X     X     X X       X             X X   X   X       \n"
		"X     X   X X X     X   X X   X   X     X   X X       X       X X     X X X X   X               X   X X X X X X X X X   X       X   X       X X       X     X     X     X X X         X X   X   X     X     X X X X X X X \n"
		"  X             X   X               X   X X   X     X X X X X X X             X       X X   X X X                       X   X X   X   X       X X X X X X         X X           X   X X         X   X   X     X X   X X   \n"
		"X       X X X X   X       X   X X X X       X   X X X   X   X X X   X   X   X   X X       X     X X X X X X X X X X X X X X           X X   X X     X     X X X X   X       X     X X X X X     X X X     X X X       X   \n"
		"X   X X X     X   X X X X X   X X     X   X         X   X   X         X X X     X X X   X X         X         X   X X       X X   X     X     X       X         X X X X X     X   X X X X X   X   X       X   X X X X   X \n"
		"X X X X X   X         X     X     X X   X X X X   X     X X X   X X   X X   X   X   X X X X         X X     X   X X                   X   X X     X   X   X X   X X         X   X     X X         X X     X X   X X   X X \n"
		"      X X X   X     X X X X X   X X   X X     X   X X X X         X X   X     X X X   X X       X   X X X X     X X X X   X X   X             X X X         X X   X X X   X     X   X   X X     X X       X       X       \n"
		"    X X     X X X           X X     X X   X X X X X X X X X X X   X       X X X X     X X   X X X   X   X X X       X X     X X X X X X X   X X   X     X X         X X     X X X   X   X   X X   X   X   X X   X   X     \n"
		"  X X   X X   X X       X     X X       X     X       X X X   X X X           X X   X X             X   X X         X   X X X   X X   X X X   X X X       X X X     X X X X   X X X   X X X     X         X     X       X \n"
		"  X X X     X     X         X X   X   X X X X X   X   X     X       X   X   X     X               X   X X   X X   X X     X X X           X X     X X         X       X     X     X   X X   X X           X X   X X X     \n"
		"X   X   X     X X     X   X X   X   X X X         X       X X                       X   X       X X X   X       X       X   X     X   X   X     X     X X   X   X   X X X X   X X X X X     X X X X X X X     X X   X X   \n"
		"X X   X X   X       X X     X     X X X   X X   X X X X     X X   X X X     X   X     X   X X X   X   X   X X   X   X X X X     X X   X     X X     X X   X       X         X       X   X   X X     X   X   X         X X \n"
		"X X X   X       X X   X     X X   X   X   X   X X X   X   X   X   X   X X                                 X   X     X     X     X   X X   X   X     X   X X         X   X X     X X   X X   X X X X           X   X       \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"X   X   X       X   X   X   X   X   X   X X       X X       X     X   X   X     X X     X     X X X X   X X       X   X     X   X X X     X   X X X X   X X   X X         X       X X X X X X X     X     X   X   X     X \n"
		"X X X X     X X X X   X       X X   X X   X X X X   X   X       X X   X     X X   X X     X X   X X X     X X   X X X   X   X X   X X   X X X X X X   X X X X X X X X   X X X   X X X       X   X   X     X X X X X X     \n"
		"  X     X         X X X       X   X     X     X X X   X X   X   X X           X   X X X   X X     X     X X   X X X   X X   X X       X X       X   X X   X   X     X     X     X   X       X     X X     X   X         X \n"
		"X   X   X X X       X   X     X     X X X X X   X   X   X     X   X   X   X X X   X       X X   X X X X X   X       X         X   X X X X X X X X X X           X   X   X X X   X X   X X             X   X X X X X     X \n"
		"    X X X     X X   X X   X X X       X   X     X       X     X X                     X   X X   X       X X       X X   X X X X X   X   X     X     X   X     X   X   X X     X X X X   X     X X       X X     X X     X \n"
		"X X X   X   X     X       X       X X X X X X X X       X   X     X     X X X X       X   X X   X   X X     X X X   X   X   X     X     X X X   X   X X X         X         X       X X X     X     X X X X X   X X X X   \n"
		"X X     X X         X   X X X X X   X   X       X     X           X           X X   X   X   X X   X   X X     X X X       X X X X X   X   X   X     X     X X X   X   X X           X X X   X X X       X         X X     \n"
		"X       X X X X X X X     X     X     X X   X   X X X   X X       X     X X X X   X         X X   X X   X   X X X   X X   X   X X   X   X X X X           X   X     X X X   X       X     X           X X   X X X         \n"
		"    X X X                 X X     X   X X X   X   X X X   X       X X     X         X   X X X X       X       X X X X X     X X   X X X   X   X X X X   X X       X   X X X     X X X   X       X X     X X   X     X X   \n"
		"  X X X   X X X X X X X X   X       X X     X                 X   X       X X X       X   X X   X X X X   X X   X   X             X         X X X   X X       X X X X       X X   X X   X X X     X X X     X X X     X   \n"
		"X       X X   X X X X   X X   X     X X   X           X X   X X   X X X   X   X X       X X X   X X X X   X   X     X   X   X X   X     X X     X     X X   X   X X     X X     X       X X   X               X           \n"
		"X           X   X X X X X X   X     X     X X X       X X   X     X X     X X X   X   X X   X X X X X X   X X X X       X X X   X X         X     X X X X       X X X   X X X   X X X X   X   X     X X     X X X   X     \n"
		"  X           X X     X   X X X       X       X X X     X X X X X X X     X           X     X   X   X   X     X   X   X X X   X         X X   X     X X X X X X   X X X         X X X X X     X       X X X       X     X \n"
		"    X   X   X   X     X X X X X   X X X X   X X X X X X   X   X X   X X X X X X X X   X   X   X   X   X X   X X     X X     X     X   X X X X X   X X X   X X X X         X X X   X X   X X X X   X   X     X X       X   \n"
		"X X   X X     X   X     X X       X   X X X   X X X X     X     X   X     X       X   X   X X   X       X X     X   X X   X X X X X X X             X X X X     X X X X X X       X X   X X     X     X X X           X X \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"    X X X           X   X X X       X X X     X     X X   X X   X X X         X     X     X X X X X   X X X   X X X   X       X       X X X     X     X       X     X X       X                       X X           X     \n"
		"X X X   X X X     X   X X   X X X X   X     X X X X   X X X   X   X X X X   X     X X     X   X   X X X     X X   X X       X X     X     X X X   X X   X X X X   X       X X   X X   X X X X X       X X   X   X X       \n"
		"    X     X   X     X   X X X X       X           X       X X X X   X X   X     X X         X X X             X X X X           X X X   X X             X X X   X   X     X   X X X   X X     X X X     X X   X X X X X   \n"
		"      X X X X X   X X     X X X X     X X   X X     X           X           X X     X X X X X       X     X X X X   X       X   X   X X X X X   X   X X X   X   X X X X   X X   X X X     X   X   X       X X X X   X X X \n"
		"X X X   X         X X     X     X       X     X   X         X X X X   X   X     X         X       X   X X X       X X X X X   X X   X X   X       X X       X X X   X X       X X   X X       X X X       X           X X \n"
		"X X     X X X   X                 X X X     X X X   X X X X X   X X X X   X X   X   X X X   X X   X   X   X X   X     X X X   X         X   X X       X     X X       X     X X       X     X X X X   X     X X   X X   X \n"
		"  X X   X     X   X   X X   X   X   X           X     X       X   X   X   X   X X X   X X   X X   X X X   X   X   X X X X X       X X X       X     X   X   X X X     X       X X X X   X X X   X   X   X X     X X X   X \n"
		"X X   X X X X X X   X   X X   X   X         X X X X   X       X X           X   X X     X     X X     X X X X         X     X X   X     X   X X X X       X         X X   X X X X X     X X   X X     X     X X X     X X \n"
		"X X     X X   X X   X X X X   X       X   X     X   X X         X   X               X     X X X   X X X X X       X   X X   X X     X   X         X X X   X   X X   X   X     X X X X X X     X X X X X X X   X X       X \n"
		"    X   X X X X   X   X   X           X X X X   X   X     X X   X   X X X   X X X     X X X X X     X X X   X X X X X       X X   X X       X X X X X             X   X X   X X X           X X X         X X X   X   X X \n"
		"X     X   X   X   X   X X   X   X   X         X X X         X   X   X X       X         X     X X X   X   X   X   X X X X X X         X X     X X   X X   X   X       X X       X X     X X X       X X       X   X     X \n"
		"X X X X     X X X   X X X     X X X     X   X X X     X       X X X   X   X X X X       X X     X X X X X   X         X   X X X X X X X   X X X X X X X   X     X X   X   X X     X X X     X X       X X   X X X     X X \n"
		"  X   X   X             X X X   X       X X   X X   X   X         X   X X X       X     X               X X   X X X X X X         X   X X X     X X X   X   X     X X         X X   X X X   X     X   X X X       X X X X \n"
		"X       X   X X   X X   X X X   X   X X X   X   X X X     X X X X X X   X X X X X     X     X X X X   X X   X       X X X X   X X       X X X   X X X X X X X       X X X X X X X X           X X       X X X         X   \n"
		"X     X X       X X X         X           X     X       X X     X X X     X   X X X   X   X       X     X     X X X X X X X   X   X X X             X   X X X     X X   X X   X X X X X X   X     X X X       X           \n"
		"X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X   X \n"
		"X     X X     X X X   X X         X X X X X       X X       X     X   X   X         X   X   X       X X   X   X     X X X X   X     X           X     X       X X X X     X     X       X X X   X X X X   X   X   X X X   \n"
		"  X X X X   X   X   X X       X   X     X   X   X X X X     X     X         X     X     X   X   X   X     X X   X X     X   X   X           X X                     X X     X   X   X   X   X X   X X     X X X X   X X   \n"
		"X X   X X X   X   X   X X X X       X X         X         X   X X X X     X       X X X X   X   X     X X             X X X X X   X   X       X X X X X     X X X X X X X X     X X X X   X X X X     X X X   X X X   X X \n"
		"X X X     X X   X   X   X X X   X X X     X X X X   X   X X X   X   X X   X X       X             X X X     X   X X   X X     X X           X X X             X X     X   X X       X X X X X X         X   X   X       X \n"
		"  X   X   X   X   X   X X X X     X X X X     X X       X X X   X X X     X     X X X X X X X X       X X     X X   X   X       X         X   X X   X X X X   X   X   X X     X     X X   X X   X X X X   X           X   \n"
		"X X X X X   X X X X   X X X   X X X         X X       X       X       X X X X X X X     X   X X X         X X       X X X X X   X     X   X X X X X   X   X     X           X X X   X X X X X X       X X X X       X X   \n"
	), true /*isPure*/, false /*tryHarder*/);
	EXPECT_TRUE(r.isValid());
	EXPECT_TRUE(r.readerInit());
	EXPECT_FALSE(r.isCompact());
	EXPECT_EQ(r.nbLayers(), 22);
}

TEST(AZDetectorTest, ReaderInitCompact)
{
	{
		// Null (not set)
		auto r = Aztec::Detect(ParseBitMatrix(
			"            X X   X         X \n"
			"    X X X X   X X X   X     X \n"
			"    X X             X   X     \n"
			"  X X X X X X X X X X X X     \n"
			"  X   X               X       \n"
			"    X X   X X X X X   X     X \n"
			"X     X   X       X   X X     \n"
			"X     X   X   X   X   X     X \n"
			"X     X   X       X   X X X   \n"
			"X X   X   X X X X X   X X X X \n"
			"    X X               X   X   \n"
			"      X X X X X X X X X X X   \n"
			"X             X X         X X \n"
			"X         X   X     X X       \n"
			"X X X X     X X X         X X \n"
		), true /*isPure*/, false /*tryHarder*/);
		EXPECT_TRUE(r.isValid());
		EXPECT_FALSE(r.readerInit());
		EXPECT_TRUE(r.isCompact());
		EXPECT_EQ(r.nbLayers(), 1);
	}
	{
		// Set
		auto r = Aztec::Detect(ParseBitMatrix(
			"            X X   X         X \n"
			"    X X X X   X X X   X     X \n"
			"    X X     X       X   X     \n"
			"  X X X X X X X X X X X X     \n"
			"  X X X               X       \n"
			"    X X   X X X X X   X X   X \n"
			"X   X X   X       X   X X     \n"
			"X     X   X   X   X   X     X \n"
			"X     X   X       X   X   X   \n"
			"X X   X   X X X X X   X   X X \n"
			"    X X               X   X   \n"
			"      X X X X X X X X X X X   \n"
			"X       X X   X   X X     X X \n"
			"X         X   X     X X       \n"
			"X X X X     X X X         X X \n"
		), true /*isPure*/, false /*tryHarder*/);
		EXPECT_TRUE(r.isValid());
		EXPECT_TRUE(r.readerInit());
		EXPECT_TRUE(r.isCompact());
		EXPECT_EQ(r.nbLayers(), 1);
	}
}

TEST(AZDetectorTest, Rune)
{
	{
		auto r = Aztec::Detect(ParseBitMatrix(
			"X X X   X   X   X   X \n"
			"X X X X X X X X X X X \n"
			"  X               X   \n"
			"X X   X X X X X   X X \n"
			"  X   X       X   X   \n"
			"X X   X   X   X   X X \n"
			"  X   X       X   X   \n"
			"X X   X X X X X   X X \n"
			"  X               X   \n"
			"  X X X X X X X X X X \n"
			"    X   X   X   X     \n"
		), false /*isPure*/, false /*tryHarder*/);

		EXPECT_TRUE(r.isValid());
		EXPECT_EQ(r.nbDatablocks(), 0);
		EXPECT_EQ(r.runeValue(), 0);
	}
	{
		auto r = Aztec::Detect(ParseBitMatrix(
			"X X X   X X     X   X \n"
			"X X X X X X X X X X X \n"
			"  X               X X \n"
			"  X   X X X X X   X X \n"
			"  X   X       X   X   \n"
			"X X   X   X   X   X X \n"
			"X X   X       X   X X \n"
			"X X   X X X X X   X   \n"
			"X X               X X \n"
			"  X X X X X X X X X X \n"
			"    X     X           \n"
		), true /*isPure*/, false /*tryHarder*/);

		EXPECT_TRUE(r.isValid());
		EXPECT_EQ(r.nbDatablocks(), 0);
		EXPECT_EQ(r.runeValue(), 25);
	}
	{
		auto r = Aztec::Detect(ParseBitMatrix(
			"X X             X   X \n"
			"X X X X X X X X X X X \n"
			"  X               X   \n"
			"X X   X X X X X   X   \n"
			"  X   X       X   X X \n"
			"  X   X   X   X   X   \n"
			"  X   X       X   X X \n"
			"  X   X X X X X   X X \n"
			"X X               X   \n"
			"  X X X X X X X X X X \n"
			"          X X         \n"
		), true /*isPure*/, false /*tryHarder*/);

		// This is just the core of a regular compact code, and not a valid rune
		EXPECT_FALSE(r.isValid());
	}
}