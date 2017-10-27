#include "gtest/gtest.h"
#include "aztec/AZDetector.h"
#include "aztec/AZDetectorResult.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "BitMatrixUtility.h"
#include "PseudoRandom.h"

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

	// Rotates a square BitMatrix to the right by 90 degrees
	void RotateRight(BitMatrix& input) {
		int width = input.width();
		BitMatrix result(width);
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < width; y++) {
				if (input.get(x, y)) {
					result.set(y, width - x - 1);
				}
			}
		}
		input = std::move(result);
	}

	// Returns the transpose of a bit matrix, which is equivalent to rotating the
	// matrix to the right, and then flipping it left-to-right
	void Transpose(BitMatrix& input) {
		int width = input.width();
		BitMatrix result(width);
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < width; y++) {
				if (input.get(x, y)) {
					result.set(y, x);
				}
			}
		}
		input = std::move(result);
	}

	// Zooms a bit matrix so that each bit is factor x factor
	BitMatrix MakeLarger(const BitMatrix& input, int factor) {
		int width = input.width();
		BitMatrix output(width * factor);
		for (int inputY = 0; inputY < width; inputY++) {
			for (int inputX = 0; inputX < width; inputX++) {
				if (input.get(inputX, inputY)) {
					output.setRegion(inputX * factor, inputY * factor, factor, factor);
				}
			}
		}
		return output;
	}

	// Test that we can tolerate errors in the parameter locator bits
	void TestErrorInParameterLocator(const std::string& data, int nbLayers, bool isCompact, const BitMatrix &matrix_)
	{
		PseudoRandom random(std::hash<std::string>()(data));
		auto orientationPoints = GetOrientationPoints(matrix_, isCompact);
		for (bool isMirror : { false, true }) {
			BitMatrix matrix = matrix_.copy();
			for (int i = 0; i < 4; ++i) {
				// Systematically try every possible 1- and 2-bit error.
				for (int error1 = 0; error1 < (int)orientationPoints.size(); error1++) {
					for (int error2 = error1; error2 < (int)orientationPoints.size(); error2++) {
						BitMatrix copy = matrix.copy();
						if (isMirror) {
							Transpose(copy);
						}
						copy.flip(orientationPoints[error1].x, orientationPoints[error1].y);
						if (error2 > error1) {
							// if error2 == error1, we only test a single error
							copy.flip(orientationPoints[error2].x, orientationPoints[error2].y);
						}
						// The detector doesn't seem to work when matrix bits are only 1x1.  So magnify.
						Aztec::DetectorResult r = Aztec::Detector::Detect(MakeLarger(copy, 3), isMirror);
						EXPECT_EQ(r.isValid(), true);
						EXPECT_EQ(r.nbLayers(), nbLayers);
						EXPECT_EQ(r.isCompact(), isCompact);
						//DecoderResult res = new Decoder().decode(r);
						//assertEquals(data, res.getText());
					}
				}
				// Try a few random three-bit errors;
				for (int i = 0; i < 5; i++) {
					BitMatrix copy = matrix.copy();
					std::set<size_t> errors;
					while (errors.size() < 3) {
						errors.insert(random.next(size_t(0), orientationPoints.size() - 1));
					}
					for (int error : errors) {
						copy.flip(orientationPoints[error].x, orientationPoints[error].y);
					}
					Aztec::DetectorResult r = Aztec::Detector::Detect(MakeLarger(copy, 3), false);
					EXPECT_EQ(r.isValid(), false);
				}

				RotateRight(matrix);
			}
		}
	}
} // anonymous

TEST(AZDetectorTest, ErrorInParameterLocatorZeroZero)
{
	// Layers=1, CodeWords=1.  So the parameter info and its Reed-Solomon info
	// will be completely zero!
	TestErrorInParameterLocator("X", 1, true, Utility::ParseBitMatrix(
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

TEST(AZDetectorTest, ErrorInParameterLocatorCompact)
{
	TestErrorInParameterLocator("This is an example Aztec symbol for Wikipedia.", 3, true, Utility::ParseBitMatrix(
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

TEST(AZDetectorTest, ErrorInParameterLocatorNotCompact)
{
	std::string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwxyz";
	TestErrorInParameterLocator(alphabet + alphabet + alphabet, 6, false, Utility::ParseBitMatrix(
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
