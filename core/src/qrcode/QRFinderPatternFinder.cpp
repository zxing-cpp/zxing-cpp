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

#include "qrcode/QRFinderPatternFinder.h"
#include "qrcode/QRFinderPatternInfo.h"
#include "BitMatrix.h"
#include "DecodeHints.h"
#include "DecodeStatus.h"
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
static const int MAX_MODULES = 97; // support up to version 20 for mobile clients

using StateCount = FinderPatternFinder::StateCount;

/**
* Given a count of black/white/black/white/black pixels just seen and an end position,
* figures the location of the center of this run.
*/
static float CenterFromEnd(const StateCount& stateCount, int end)
{
	return (float)(end - stateCount[4] - stateCount[3]) - stateCount[2] / 2.0f;
}

/**
 * @param stateCount count of black/white/black/white/black pixels just read
 * @return true iff the proportions of the counts is close enough to the 1/1/3/1/1 ratios
 *         used by finder patterns to be considered a match
 */
static bool FoundPatternDiagonal(const StateCount& stateCount) {
	int totalModuleSize = 0;
	for (int i = 0; i < 5; i++) {
		int count = stateCount[i];
		if (count == 0) {
			return false;
		}
		totalModuleSize += count;
	}
	if (totalModuleSize < 7) {
		return false;
	}
	float moduleSize = totalModuleSize / 7.0f;
	float maxVariance = moduleSize / 1.333f;
	// Allow less than 75% variance from 1-1-3-1-1 proportions
	return
		std::abs(moduleSize - stateCount[0]) < maxVariance &&
		std::abs(moduleSize - stateCount[1]) < maxVariance &&
		std::abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
		std::abs(moduleSize - stateCount[3]) < maxVariance &&
		std::abs(moduleSize - stateCount[4]) < maxVariance;
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
	int stateCountTotal = Accumulate(stateCount, 0);
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	return FinderPatternFinder::FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN();
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
	int stateCountTotal = Accumulate(stateCount, 0);
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	return FinderPatternFinder::FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, j) : std::numeric_limits<float>::quiet_NaN();
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
				return static_cast<int>(std::abs(firstConfirmedCenter->x() - center.x()) - std::abs(firstConfirmedCenter->y() - center.y())) / 2;
			}
		}
	}
	return 0;
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
	//int max = possibleCenters.size();
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
	float average = totalModuleSize / static_cast<float>(possibleCenters.size());
	float totalDeviation = 0.0f;
	for (const FinderPattern& pattern : possibleCenters) {
		totalDeviation += std::abs(pattern.estimatedModuleSize() - average);
	}
	return totalDeviation <= 0.05f * totalModuleSize;
}

/**
* <p>Orders by furthest from average</p>
*/
struct FurthestFromAverageComparator
{
	float average;

	bool operator()(const FinderPattern& center1, const FinderPattern& center2) const {
		return std::abs(center1.estimatedModuleSize() - average) > std::abs(center2.estimatedModuleSize() - average);
	}
};

/**
* <p>Orders by {@link FinderPattern#getCount()}, descending.</p>
*/
struct CenterComparator
{
	float average;

	bool operator()(const FinderPattern& center1, const FinderPattern& center2) const {
		if (center2.count() == center1.count()) {
			return std::abs(center1.estimatedModuleSize() - average) < std::abs(center2.estimatedModuleSize() - average);
		}
		else {
			return center1.count() > center2.count();
		}
	}
};


/**
* @return the 3 best {@link FinderPattern}s from our list of candidates. The "best" are
*         those that have been detected at least {@link #CENTER_QUORUM} times, and whose module
*         size differs from the average among those patterns the least
* @throws NotFoundException if 3 such finder patterns do not exist
*/
static bool SelectBestPatterns(std::vector<FinderPattern>& possibleCenters)
{
	int startSize = static_cast<int>(possibleCenters.size());
	if (startSize < 3) {
		// Couldn't find enough finder patterns
		return false;
	}

	// Filter outlier possibilities whose module size is too different
	if (startSize > 3) {
		// But we can only afford to do so if we have at least 4 possibilities to choose from
		float totalModuleSize = 0.0f;
		float square = 0.0f;
		for (const FinderPattern& center : possibleCenters) {
			float size = center.estimatedModuleSize();
			totalModuleSize += size;
			square += size * size;
		}
		float average = totalModuleSize / static_cast<float>(startSize);
		float stdDev = std::sqrt(square / startSize - average * average);

		std::sort(possibleCenters.begin(), possibleCenters.end(), FurthestFromAverageComparator{ average });

		float limit = std::max(0.2f * average, stdDev);

		for (int i = 0; i < int(possibleCenters.size()) && possibleCenters.size() > 3; i++) {
			const FinderPattern& pattern = possibleCenters[i];
			if (std::abs(pattern.estimatedModuleSize() - average) > limit) {
				possibleCenters.erase(possibleCenters.begin() + i);
				i--;
			}
		}
	}

	if (possibleCenters.size() > 3) {
		// Throw away all but those first size candidate points we found.

		float totalModuleSize = 0.0f;
		for (const FinderPattern& possibleCenter : possibleCenters) {
			totalModuleSize += possibleCenter.estimatedModuleSize();
		}

		float average = totalModuleSize / static_cast<float>(possibleCenters.size());

		std::sort(possibleCenters.begin(), possibleCenters.end(), CenterComparator{ average });

		possibleCenters.resize(3);
	}
	return true;
}

/**
* Returns the z component of the cross product between vectors BC and BA.
*/
static float CrossProductZ(const ResultPoint& a, const ResultPoint& b, const ResultPoint& c)
{
	return (c.x() - b.x())*(a.y() - b.y()) - (c.y() - b.y())*(a.x() - b.x());
}

/**
* Orders an array of three ResultPoints in an order [A,B,C] such that AB is less than AC
* and BC is less than AC, and the angle between BC and BA is less than 180 degrees.
*
* @param patterns array of three {@code ResultPoint} to order
*/
static void OrderBestPatterns(std::vector<FinderPattern>& patterns)
{
	assert(patterns.size() == 3);

	auto &p0 = patterns[0], &p1 = patterns[1], &p2 = patterns[2];

	// Find distances between pattern centers
	float zeroOneDistance = ResultPoint::Distance(p0, p1);
	float oneTwoDistance = ResultPoint::Distance(p1, p2);
	float zeroTwoDistance = ResultPoint::Distance(p0, p2);

	// Assume one closest to other two is B; A and C will just be guesses at first
	if (oneTwoDistance >= zeroOneDistance && oneTwoDistance >= zeroTwoDistance)
		std::swap(p0, p1);
	else if (zeroTwoDistance >= oneTwoDistance && zeroTwoDistance >= zeroOneDistance)
		; // do nothing, the order is correct
	else
		std::swap(p1, p2);

	// Use cross product to figure out whether A and C are correct or flipped.
	// This asks whether BC x BA has a positive z component, which is the arrangement
	// we want for A, B, C. If it's negative, then we've got it flipped around and
	// should swap A and C.
	if (CrossProductZ(p0, p1, p2) < 0.0f) {
		std::swap(p0, p2);
	}
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
							// Clear state to start looking again
							currentState = 0;
							stateCount.fill(0);
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
		if (FinderPatternFinder::FoundPatternCross(stateCount)) {
			bool confirmed = FinderPatternFinder::HandlePossibleCenter(image, stateCount, i, maxJ, possibleCenters);
			if (confirmed) {
				iSkip = stateCount[0];
				if (hasSkipped) {
					// Found a third one
					done = HaveMultiplyConfirmedCenters(possibleCenters);
				}
			}
		}
	}

	if (!SelectBestPatterns(possibleCenters))
		return {};

	OrderBestPatterns(possibleCenters);

	return {possibleCenters[0], possibleCenters[1], possibleCenters[2]};
}


/**
* @param stateCount count of black/white/black/white/black pixels just read
* @return true iff the proportions of the counts is close enough to the 1/1/3/1/1 ratios
*         used by finder patterns to be considered a match
*/
bool
FinderPatternFinder::FoundPatternCross(const StateCount& stateCount) {
	int totalModuleSize = Accumulate(stateCount, 0);
	if (totalModuleSize < 7) {
		return false;
	}
	float moduleSize = totalModuleSize / 7.0f;
	float maxVariance = moduleSize / 2.0f;
	// Allow less than 50% variance from 1-1-3-1-1 proportions
	return
		std::abs(moduleSize - stateCount[0]) < maxVariance &&
		std::abs(moduleSize - stateCount[1]) < maxVariance &&
		std::abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
		std::abs(moduleSize - stateCount[3]) < maxVariance &&
		std::abs(moduleSize - stateCount[4]) < maxVariance;
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
* @param i row where finder pattern may be found
* @param j end of possible finder pattern in row
* @param pureBarcode true if in "pure barcode" mode
* @return true if a finder pattern candidate was found this time
*/
bool
FinderPatternFinder::HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j, std::vector<FinderPattern>& possibleCenters)
{
	int stateCountTotal = Accumulate(stateCount, 0);
	float centerJ = CenterFromEnd(stateCount, j);
	float centerI = CrossCheckVertical(image, i, static_cast<int>(centerJ), stateCount[2], stateCountTotal);
	if (std::isnan(centerI))
		return false;

	// Re-cross check
	centerJ = CrossCheckHorizontal(image, static_cast<int>(centerJ), static_cast<int>(centerI), stateCount[2],
								   stateCountTotal);
	if (std::isnan(centerJ) || !CrossCheckDiagonal(image, (int)centerI, (int)centerJ))
		return false;

	float estimatedModuleSize = stateCountTotal / 7.0f;
	auto center = ZXing::FindIf(possibleCenters, [=](const FinderPattern& center) {
		return center.aboutEquals(estimatedModuleSize, centerI, centerJ);
	});
	if (center != possibleCenters.end())
		*center = center->combineEstimate(centerI, centerJ, estimatedModuleSize);
	else
		possibleCenters.emplace_back(centerJ, centerI, estimatedModuleSize);

	return true;
}

} // QRCode
} // ZXing
