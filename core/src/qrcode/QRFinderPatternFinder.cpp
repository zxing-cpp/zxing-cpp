/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "QRFinderPatternFinder.h"
#include "QRFinderPatternInfo.h"
#include "BitMatrix.h"
#include "ZXContainerAlgorithms.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>

namespace ZXing {
namespace QRCode {

static const int CENTER_QUORUM = 2;
static const int MIN_SKIP = 3; // 1 pixel/module times 3 modules/center
static const int MAX_MODULES = 20 * 4 + 17; // support up to version 20 for mobile clients

using StateCount = std::array<int, 5>;

/**
* Given a count of black/white/black/white/black pixels just seen and an end position,
* figures the location of the center of this run.
*/
static float CenterFromEnd(const StateCount& stateCount, int end)
{
	float a = stateCount[4] + stateCount[3] + stateCount[2] / 2.f;
	float b = stateCount[4] + (stateCount[3] + stateCount[2] + stateCount[1]) / 2.f;
	float c = (stateCount[4] + stateCount[3] + stateCount[2] + stateCount[1] + stateCount[0]) / 2.f;
	return end - (2 * a + b + c) / 4;
}

/**
* @param stateCount count of black/white/black/white/black pixels just read
* @param similarityThreshold allow less than "threshold" variance from 1-1-3-1-1 proportions
* @return true iff the proportions of the counts is close enough to the 1/1/3/1/1 ratios
*         used by finder patterns to be considered a match
*/
static bool FoundPattern(const StateCount& stateCount, float similarityThreshold)
{
	int totalModuleSize = Reduce(stateCount);
	if (totalModuleSize < 7) {
		return false;
	}
	float moduleSize = totalModuleSize / 7.0f;
	float maxVariance = moduleSize * similarityThreshold;
	return
		std::abs(moduleSize - stateCount[0]) < maxVariance &&
		std::abs(moduleSize - stateCount[1]) < maxVariance &&
		std::abs(3 * moduleSize - stateCount[2]) < 3 * maxVariance &&
		std::abs(moduleSize - stateCount[3]) < maxVariance &&
		std::abs(moduleSize - stateCount[4]) < maxVariance;
}

static bool FoundPatternCross(const StateCount& stateCount)
{
	return FoundPattern(stateCount, 0.5f);
}

static bool FoundPatternDiagonal(const StateCount& stateCount)
{
	return FoundPattern(stateCount, 0.75f);
}

/**
 * After a vertical and horizontal scan finds a potential finder pattern, this method
 * "cross-cross-cross-checks" by scanning down diagonally through the center of the possible
 * finder pattern to see if the same proportion is detected.
 *
 * @param centerI row where a finder pattern was detected
 * @param centerJ center of the section that appears to cross a finder pattern
 * @param maxCount maximum reasonable number of modules that should be
 *  observed in any reading state, based on the results of the horizontal scan
 * @param originalStateCountTotal The original state count total.
 * @return true if proportions are withing expected limits
 */
static bool CrossCheckDiagonal(const BitMatrix& image, int centerI, int centerJ)
{
	StateCount stateCount = {};

	// Start counting up, left from center finding black center mass
	int i = 0;
	while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
		stateCount[2]++;
		i++;
	}
	if (stateCount[2] == 0) {
		return false;
	}

	// Continue up, left finding white space
	while (centerI >= i && centerJ >= i && !image.get(centerJ - i, centerI - i)) {
		stateCount[1]++;
		i++;
	}
	if (stateCount[1] == 0) {
		return false;
	}

	// Continue up, left finding black border
	while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
		stateCount[0]++;
		i++;
	}
	if (stateCount[0] == 0) {
		return false;
	}

	int maxI = image.height();
	int maxJ = image.width();

	// Now also count down, right from center
	i = 1;
	while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
		stateCount[2]++;
		i++;
	}

	while (centerI + i < maxI && centerJ + i < maxJ && !image.get(centerJ + i, centerI + i)) {
		stateCount[3]++;
		i++;
	}
	if (stateCount[3] == 0) {
		return false;
	}

	while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
		stateCount[4]++;
		i++;
	}
	if (stateCount[4] == 0) {
		return false;
	}

	return FoundPatternDiagonal(stateCount);
}

/**
* <p>After a horizontal scan finds a potential finder pattern, this method
* "cross-checks" by scanning down vertically through the center of the possible
* finder pattern to see if the same proportion is detected.</p>
*
* @param startI row where a finder pattern was detected
* @param centerJ center of the section that appears to cross a finder pattern
* @param maxCount maximum reasonable number of modules that should be
* observed in any reading state, based on the results of the horizontal scan
* @return vertical center of finder pattern, or {@link Float#NaN} if not found
*/
static float CrossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount, int originalStateCountTotal)
{
	StateCount stateCount = {};
	int maxI = image.height();

	// Start counting up from center
	int i = startI;
	while (i >= 0 && image.get(centerJ, i)) {
		stateCount[2]++;
		i--;
	}
	if (i < 0) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i >= 0 && !image.get(centerJ, i) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		i--;
	}
	// If already too many modules in this state or ran off the edge:
	if (i < 0 || stateCount[1] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i >= 0 && image.get(centerJ, i) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		i--;
	}
	if (stateCount[0] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	// Now also count down from center
	i = startI + 1;
	while (i < maxI && image.get(centerJ, i)) {
		stateCount[2]++;
		i++;
	}
	if (i == maxI) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i < maxI && !image.get(centerJ, i) && stateCount[3] < maxCount) {
		stateCount[3]++;
		i++;
	}
	if (i == maxI || stateCount[3] >= maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i < maxI && image.get(centerJ, i) && stateCount[4] < maxCount) {
		stateCount[4]++;
		i++;
	}
	if (stateCount[4] >= maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	// If we found a finder-pattern-like section, but its size is more than 40% different than
	// the original, assume it's a false positive
	int stateCountTotal = Reduce(stateCount);
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN();
}

/**
* <p>Like {@link #crossCheckVertical(int, int, int, int)}, and in fact is basically identical,
* except it reads horizontally instead of vertically. This is used to cross-cross
* check a vertical cross check and locate the real center of the alignment pattern.</p>
*/
static float CrossCheckHorizontal(const BitMatrix& image, int startJ, int centerI, int maxCount, int originalStateCountTotal)
{
	StateCount stateCount = {};
	int maxJ = image.width();

	int j = startJ;
	while (j >= 0 && image.get(j, centerI)) {
		stateCount[2]++;
		j--;
	}
	if (j < 0) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (j >= 0 && !image.get(j, centerI) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		j--;
	}
	if (j < 0 || stateCount[1] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (j >= 0 && image.get(j, centerI) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		j--;
	}
	if (stateCount[0] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	j = startJ + 1;
	while (j < maxJ && image.get(j, centerI)) {
		stateCount[2]++;
		j++;
	}
	if (j == maxJ) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (j < maxJ && !image.get(j, centerI) && stateCount[3] < maxCount) {
		stateCount[3]++;
		j++;
	}
	if (j == maxJ || stateCount[3] >= maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (j < maxJ && image.get(j, centerI) && stateCount[4] < maxCount) {
		stateCount[4]++;
		j++;
	}
	if (stateCount[4] >= maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	// If we found a finder-pattern-like section, but its size is significantly different than
	// the original, assume it's a false positive
	int stateCountTotal = Reduce(stateCount);
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	return FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, j) : std::numeric_limits<float>::quiet_NaN();
}

/**
* @return number of rows we could safely skip during scanning, based on the first
*         two finder patterns that have been located. In some cases their position will
*         allow us to infer that the third pattern must lie below a certain point farther
*         down in the image.
*/
static int FindRowSkip(const std::vector<FinderPattern>& possibleCenters, bool& hasSkipped)
{
	if (possibleCenters.size() <= 1) {
		return 0;
	}

	const ResultPoint* firstConfirmedCenter = nullptr;
	for (const FinderPattern& center : possibleCenters) {
		if (center.count() >= CENTER_QUORUM) {
			if (firstConfirmedCenter == nullptr) {
				firstConfirmedCenter = &center;
			}
			else {
				// We have two confirmed centers
				// How far down can we skip before resuming looking for the next
				// pattern? In the worst case, only the difference between the
				// difference in the x / y coordinates of the two centers.
				// This is the case where you find top left last.
				hasSkipped = true;
				auto diff = *firstConfirmedCenter - center;
				return static_cast<int>(std::abs(diff.x) - std::abs(diff.y)) / 2;
			}
		}
	}
	return 0;
}

/**
* <p>This is called when a horizontal scan finds a possible alignment pattern. It will
* cross check with a vertical scan, and if successful, will, ah, cross-cross-check
* with another horizontal scan. This is needed primarily to locate the real horizontal
* center of the pattern in cases of extreme skew.
* And then we cross-cross-cross check with another diagonal scan.</p>
*
* <p>If that succeeds the finder pattern location is added to a list that tracks
* the number of times each location has been nearly-matched as a finder pattern.
* Each additional find is more evidence that the location is in fact a finder
* pattern center
*
* @param stateCount reading state module counts from horizontal scan
* @param y row where finder pattern may be found
* @param x end of possible finder pattern in row
* @param possibleCenters [in/out] current list of centers to be updated
* @return true if a finder pattern candidate was found this time
*/
static bool HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int y, int x, std::vector<FinderPattern>& possibleCenters)
{
	int stateCountTotal = Reduce(stateCount);
	float centerX = CenterFromEnd(stateCount, x);
	constexpr auto off = 0.49f;
	float centerY = CrossCheckVertical(image, y, static_cast<int>(centerX + off), stateCount[2], stateCountTotal);
	if (std::isnan(centerY))
		return false;

	// Re-cross check
	centerX = CrossCheckHorizontal(image, static_cast<int>(centerX + off), static_cast<int>(centerY + off), stateCount[2],
								   stateCountTotal);
	if (std::isnan(centerX) || !CrossCheckDiagonal(image, static_cast<int>(centerY + off), static_cast<int>(centerX + off)))
		return false;

	float estimatedModuleSize = stateCountTotal / 7.0f;
	auto center = ZXing::FindIf(possibleCenters, [=](const FinderPattern& center) {
        return center.aboutEquals(estimatedModuleSize, centerY, centerX);
    });
	if (center != possibleCenters.end())
		*center = center->combineEstimate(centerY, centerX, estimatedModuleSize);
	else
		possibleCenters.emplace_back(centerX, centerY, estimatedModuleSize);

	return true;
}

/**
* @return true iff we have found at least 3 finder patterns that have been detected
*         at least {@link #CENTER_QUORUM} times each, and, the estimated module size of the
*         candidates is "pretty similar"
*/
static bool HaveMultiplyConfirmedCenters(const std::vector<FinderPattern>& possibleCenters)
{
	int confirmedCount = 0;
	float totalModuleSize = 0.0f;
	for (const FinderPattern& pattern : possibleCenters) {
		if (pattern.count() >= CENTER_QUORUM) {
			confirmedCount++;
			totalModuleSize += pattern.estimatedModuleSize();
		}
	}
	if (confirmedCount < 3) {
		return false;
	}
	// OK, we have at least 3 confirmed centers, but, it's possible that one is a "false positive"
	// and that we need to keep looking. We detect this by asking if the estimated module sizes
	// vary too much. We arbitrarily say that when the total deviation from average exceeds
	// 5% of the total module size estimates, it's too much.
	float totalDeviation =
		Reduce(possibleCenters, 0.f, [average = totalModuleSize / possibleCenters.size()](auto sum, auto pattern) {
			return sum + std::abs(pattern.estimatedModuleSize() - average);
		});

	return totalDeviation <= 0.05f * totalModuleSize;
}

/**
* @return the 3 best {@link FinderPattern}s from our list of candidates. The "best" are
*         those have similar module size and form a shape closer to a isosceles right triangle.
*         Return invalid if 3 such finder patterns do not exist.
*/
static FinderPatternInfo SelectBestPatterns(std::vector<FinderPattern> possibleCenters)
{
	int nbPossibleCenters = Size(possibleCenters);
	if (nbPossibleCenters < 3) {
		// Couldn't find enough finder patterns
		return {};
	}

	std::sort(possibleCenters.begin(), possibleCenters.end(), [](const auto& a, const auto& b) {
		return a.estimatedModuleSize() < b.estimatedModuleSize();
	});

	double distortion = std::numeric_limits<double>::max();
	std::array<FinderPattern, 3> bestPatterns;
	std::array<double, 3> squares;

	auto squaredDistance = [](PointF a, PointF b) { return dot((a - b), (a - b)); };

	for (int i = 0; i < nbPossibleCenters - 2; i++) {
		auto& fpi = possibleCenters[i];
		float minModuleSize = fpi.estimatedModuleSize();

		for (int j = i + 1; j < nbPossibleCenters - 1; j++) {
			auto& fpj = possibleCenters[j];
			double squares0 = squaredDistance(fpi, fpj);

			for (int k = j + 1; k < nbPossibleCenters; k++) {
				auto& fpk = possibleCenters[k];
				float maxModuleSize = fpk.estimatedModuleSize();
				if (maxModuleSize > minModuleSize * 1.4f) {
					// module size is not similar. since we sorted the input, we can break the inner loop
					break;
				}

				squares[0] = squares0;
				squares[1] = squaredDistance(fpj, fpk);
				squares[2] = squaredDistance(fpi, fpk);
				std::sort(squares.begin(), squares.end());

				// a^2 + b^2 = c^2 (Pythagorean theorem), and a = b (isosceles triangle).
				// Since any right triangle satisfies the formula c^2 - b^2 - a^2 = 0,
				// we need to check both two equal sides separately.
				// The value of |c^2 - 2 * b^2| + |c^2 - 2 * a^2| increases as dissimilarity
				// from isosceles right triangle.
				double d = std::abs(squares[2] - 2 * squares[1]) + std::abs(squares[2] - 2 * squares[0]);
				if (d < distortion) {
					distortion = d;
					bestPatterns = {fpi, fpj, fpk};
				}
			}
		}
	}

	// Orders an array of three Points in an order [A,B,C] such that AB is less than AC
	// and BC is less than AC, and the angle between BC and BA is less than 180 degrees.
	// TODO: c++17
	auto &a = bestPatterns[0], &b = bestPatterns[1], &c = bestPatterns[2];

	if (!a.isValid() && b.isValid() && c.isValid())
		return {};

	// Find distances between pattern centers
	auto distAB = distance(a, b);
	auto distBC = distance(b, c);
	auto distAC = distance(a, c);

	// Assume one closest to other two is B; A and C will just be guesses at first
	if (distBC >= distAB && distBC >= distAC)
		std::swap(a, b);
	else if (distAC >= distBC && distAC >= distAB)
		; // do nothing, the order is correct
	else
		std::swap(b, c);

	// Use cross product to figure out whether A and C are correct or flipped.
	// This asks whether BC x BA has a positive z component, which is the arrangement
	// we want for A, B, C. If it's negative, then we've got it flipped around and
	// should swap A and C.
	if (cross(c - b, a - b) < 0) {
		std::swap(a, c);
	}

	return {a, b, c};
}

FinderPatternInfo FinderPatternFinder::Find(const BitMatrix& image, bool tryHarder)
{
	int maxI = image.height();
	int maxJ = image.width();
	// We are looking for black/white/black/white/black modules in
	// 1:1:3:1:1 ratio; this tracks the number of such modules seen so far

	// Let's assume that the maximum version QR Code we support takes up 1/4 the height of the
	// image, and then account for the center being 3 modules in size. This gives the smallest
	// number of pixels the center could be, so skip this often. When trying harder, look for all
	// QR versions regardless of how dense they are.
	int iSkip = (3 * maxI) / (4 * MAX_MODULES);
	if (iSkip < MIN_SKIP || tryHarder) {
		iSkip = MIN_SKIP;
	}

	bool hasSkipped = false;
	std::vector<FinderPattern> possibleCenters;

	bool done = false;
	for (int i = iSkip - 1; i < maxI && !done; i += iSkip) {
		// Get a row of black/white values
		StateCount stateCount = {};
		size_t currentState = 0;
		for (int j = 0; j < maxJ; j++) {
			if (image.get(j, i)) {
				// Black pixel
				if ((currentState & 1) == 1) { // Counting white pixels
					currentState++;
				}
				stateCount[currentState]++;
			}
			else { // White pixel
				if ((currentState & 1) == 0) { // Counting black pixels
					if (currentState == 4) { // A winner?
						if (FoundPatternCross(stateCount)) { // Yes
							bool confirmed = HandlePossibleCenter(image, stateCount, i, j, possibleCenters);
							if (confirmed) {
								// Start examining every other line. Checking each line turned out to be too
								// expensive and didn't improve performance.
								iSkip = 2;
								if (hasSkipped) {
									done = HaveMultiplyConfirmedCenters(possibleCenters);
								}
								else {
									int rowSkip = FindRowSkip(possibleCenters, hasSkipped);
									if (rowSkip > stateCount[2]) {
										// Skip rows between row of lower confirmed center
										// and top of presumed third confirmed center
										// but back up a bit to get a full chance of detecting
										// it, entire width of center of finder pattern

										// Skip by rowSkip, but back off by stateCount[2] (size of last center
										// of pattern we saw) to be conservative, and also back off by iSkip which
										// is about to be re-added
										i += rowSkip - stateCount[2] - iSkip;
										j = maxJ - 1;
									}
								}
								// Clear state to start looking again
								currentState = 0;
								stateCount.fill(0);
							}
							else {
								stateCount[0] = stateCount[2];
								stateCount[1] = stateCount[3];
								stateCount[2] = stateCount[4];
								stateCount[3] = 1;
								stateCount[4] = 0;
								currentState = 3;
								continue;
							}
						}
						else { // No, shift counts back by two
							stateCount[0] = stateCount[2];
							stateCount[1] = stateCount[3];
							stateCount[2] = stateCount[4];
							stateCount[3] = 1;
							stateCount[4] = 0;
							currentState = 3;
						}
					}
					else {
						stateCount[++currentState]++;
					}
				}
				else { // Counting white pixels
					stateCount[currentState]++;
				}
			}
		}
		if (FoundPatternCross(stateCount)) {
			bool confirmed = HandlePossibleCenter(image, stateCount, i, maxJ, possibleCenters);
			if (confirmed) {
				iSkip = stateCount[0];
				if (hasSkipped) {
					// Found a third one
					done = HaveMultiplyConfirmedCenters(possibleCenters);
				}
			}
		}
	}

	return SelectBestPatterns(possibleCenters);
}

} // QRCode
} // ZXing
