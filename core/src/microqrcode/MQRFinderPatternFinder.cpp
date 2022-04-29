/*
 * Copyright 2007 ZXing authors All rights reserved.
 * Copyright 2017 KURZ Digital Solutions GmbH & Co. KG
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

#include "MQRFinderPatternFinder.h"

#include "DecodeHints.h"
#include "MQRCornerFinder.h"
#include "MQRFakeCenterCalculator.h"

#include <algorithm>

namespace ZXing::MicroQRCode {

namespace {

class CenterComparator
{
	const float averageModuleSize_;

public:
	CenterComparator(float averageModuleSize) : averageModuleSize_(averageModuleSize) {}
	bool operator()(const FinderPattern& a, const FinderPattern& b)
	{
		// N.B.: we want the result in descending order ...
		if (a.getCount() != b.getCount()) {
			return a.getCount() > b.getCount();
		} else {
			float dA = abs(a.getEstimatedModuleSize() - averageModuleSize_);
			float dB = abs(b.getEstimatedModuleSize() - averageModuleSize_);
			return dA < dB;
		}
	}
};

} // namespace

int FinderPatternFinder::CENTER_QUORUM = 4;
int FinderPatternFinder::MIN_SKIP = 3;
int FinderPatternFinder::MAX_MODULES = 17;

FinderPatternFinder::FinderPatternFinder() : crossCheckStateCount_(5) {}

std::vector<ResultPoint> FinderPatternFinder::findCorners(const BitMatrix& image, DecodeHints const& hints)
{
	const auto bestPattern = findBestPattern(image, hints);
	if (!bestPattern)
		return {};
	return getCodeEnclosingRect(image, *bestPattern);
}

std::optional<FinderPatternInfo> FinderPatternFinder::findCenters(const BitMatrix& image, DecodeHints const& hints)
{
	const auto bestPattern = findBestPattern(image, hints);
	if (!bestPattern)
		return {};
	return generatePatternInfoForPattern(image, *bestPattern);
}

std::optional<FinderPattern> FinderPatternFinder::findBestPattern(const BitMatrix& image, DecodeHints const& hints)
{
	bool tryHarder = hints.tryHarder();
	bool pureBarcode = hints.isPure();

	int maxI = image.height();
	int maxJ = image.width();

	// We are looking for black/white/black/white/black modules in
	// 1:1:3:1:1 ratio; this tracks the number of such modules seen so far

	// Let's assume that the maximum version micro QR Code we support takes up 1/8
	// the height of the image, and then account for the center being 3
	// modules in size. This gives the smallest number of pixels the center
	// could be, so skip this often. When trying harder, look for all
	// QR versions regardless of how dense they are.
	int iSkip = (1 * maxI) / (8 * MAX_MODULES);
	if (iSkip < MIN_SKIP || tryHarder) {
		iSkip = MIN_SKIP;
	}

	bool done = false;
	std::vector<int> stateCount(5);
	for (int i = iSkip - 1; i < maxI && !done; i += iSkip) {
		// Get a row of black/white values
		stateCount[0] = 0;
		stateCount[1] = 0;
		stateCount[2] = 0;
		stateCount[3] = 0;
		stateCount[4] = 0;
		int currentState = 0;
		for (int j = 0; j < maxJ; j++) {
			if (image.get(j, i)) {
				// Black pixel
				if ((currentState & 1) == 1) {
					// Counting white pixels
					currentState++;
				}
				stateCount[currentState]++;
			} else {
				// White pixel
				if ((currentState & 1) == 0) {
					// Counting black pixels
					if (currentState == 4) {
						// A winner?
						if (foundPatternCross(stateCount)) {
							// Yes
							bool confirmed = handlePossibleCenter(image, stateCount, i, j, pureBarcode);
							if (confirmed) {
								done = haveMultiplyConfirmedCenters();
							} else {
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
							stateCount[0] = 0;
							stateCount[1] = 0;
							stateCount[2] = 0;
							stateCount[3] = 0;
							stateCount[4] = 0;
						} else {
							// No, shift counts back by two
							stateCount[0] = stateCount[2];
							stateCount[1] = stateCount[3];
							stateCount[2] = stateCount[4];
							stateCount[3] = 1;
							stateCount[4] = 0;
							currentState = 3;
						}
					} else {
						stateCount[++currentState]++;
					}
				} else { // Counting white pixels
					stateCount[currentState]++;
				}
			}
		}
		if (foundPatternCross(stateCount)) {
			bool confirmed = handlePossibleCenter(image, stateCount, i, maxJ, pureBarcode);
			if (confirmed) {
				done = haveMultiplyConfirmedCenters();
			}
		}
	}

	return selectBestPattern();
}

std::vector<ResultPoint> FinderPatternFinder::getCodeEnclosingRect(const BitMatrix& image,
																   const FinderPattern& actualPattern)
{
	CornerFinder cornerFinder;
	return cornerFinder.find(image, actualPattern);
}

std::optional<FinderPatternInfo> FinderPatternFinder::generatePatternInfoForPattern(const BitMatrix& image,
																					const FinderPattern& actualPattern)
{
	std::vector<ResultPoint> results = getCodeEnclosingRect(image, actualPattern);
	if (results.empty())
		return {};

	FakeCenterCalculator calculator(actualPattern, results);

	FinderPattern fakeTopRightPattern = calculator.getTopRightCenter();
	FinderPattern fakeBottomLeftPattern = calculator.getBottomLeftCenter();

	return FinderPatternInfo(std::vector<FinderPattern>{actualPattern, fakeTopRightPattern, fakeBottomLeftPattern});
}

/**
 * Given a count of black/white/black/white/black pixels just seen and an end position,
 * figures the location of the center of this run.
 */
float FinderPatternFinder::centerFromEnd(const std::vector<int>& stateCount, int end) const
{
	return (float)(end - stateCount[4] - stateCount[3]) - stateCount[2] / 2.0f;
}

/**
 * @param stateCount count of black/white/black/white/black pixels just read
 * @return true iff the proportions of the counts is close enough to the 1/1/3/1/1 ratios
 * used by finder patterns to be considered a match
 */
bool FinderPatternFinder::foundPatternCross(const std::vector<int>& stateCount) const
{
	int totalModuleSize = 0;
	for (int i = 0; i < 5; i++) {
		if (stateCount[i] == 0) {
			return false;
		}
		totalModuleSize += stateCount[i];
	}
	if (totalModuleSize < 7) {
		return false;
	}
	float moduleSize = (float)totalModuleSize / 7.0f;
	float maxVariance = moduleSize / 2.0f;
	// Allow less than 50% variance from 1-1-3-1-1 proportions
	return std::abs(moduleSize - stateCount[0]) < maxVariance && abs(moduleSize - stateCount[1]) < maxVariance &&
		   abs(3.0f * moduleSize - stateCount[2]) < 3.0f * maxVariance &&
		   abs(moduleSize - stateCount[3]) < maxVariance && abs(moduleSize - stateCount[4]) < maxVariance;
}

std::vector<int> FinderPatternFinder::getCrossCheckStateCount()
{
	crossCheckStateCount_[0] = 0;
	crossCheckStateCount_[1] = 0;
	crossCheckStateCount_[2] = 0;
	crossCheckStateCount_[3] = 0;
	crossCheckStateCount_[4] = 0;
	return crossCheckStateCount_;
}

/**
 * After a vertical and horizontal scan finds a potential finder pattern, this method
 * "cross-cross-cross-checks" by scanning down diagonally through the center of the possible
 * finder pattern to see if the same proportion is detected.
 *
 * @param image
 * @param startI                  row where a finder pattern was detected
 * @param centerJ                 center of the section that appears to cross a finder pattern
 * @param maxCount                maximum reasonable number of modules that should be
 *                                observed in any reading state, based on the results of the horizontal scan
 * @param originalStateCountTotal The original state count total.
 * @return true if proportions are withing expected limits
 */
bool FinderPatternFinder::crossCheckDiagonal(const BitMatrix& image, int startI, int centerJ, int maxCount,
											 int originalStateCountTotal)
{
	std::vector<int> stateCount = getCrossCheckStateCount();

	// Start counting up, left from center finding black center mass
	int i = 0;
	while (startI >= i && centerJ >= i && image.get(centerJ - i, startI - i)) {
		stateCount[2]++;
		i++;
	}

	if (startI < i || centerJ < i) {
		return false;
	}

	// Continue up, left finding white space
	while (startI >= i && centerJ >= i && !image.get(centerJ - i, startI - i) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		i++;
	}

	// If already too many modules in this state or ran off the edge:
	if (startI < i || centerJ < i || stateCount[1] > maxCount) {
		return false;
	}

	// Continue up, left finding black border
	while (startI >= i && centerJ >= i && image.get(centerJ - i, startI - i) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		i++;
	}
	if (stateCount[0] > maxCount) {
		return false;
	}

	int maxI = image.height();
	int maxJ = image.width();

	// Now also count down, right from center
	i = 1;
	while (startI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, startI + i)) {
		stateCount[2]++;
		i++;
	}

	// Ran off the edge?
	if (startI + i >= maxI || centerJ + i >= maxJ) {
		return false;
	}

	while (startI + i < maxI && centerJ + i < maxJ && !image.get(centerJ + i, startI + i) && stateCount[3] < maxCount) {
		stateCount[3]++;
		i++;
	}

	if (startI + i >= maxI || centerJ + i >= maxJ || stateCount[3] >= maxCount) {
		return false;
	}

	while (startI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, startI + i) && stateCount[4] < maxCount) {
		stateCount[4]++;
		i++;
	}

	if (stateCount[4] >= maxCount) {
		return false;
	}

	// If we found a finder-pattern-like section, but its size is more than 100% different than
	// the original, assume it's a false positive
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	return std::abs(stateCountTotal - originalStateCountTotal) < 2 * originalStateCountTotal &&
		   foundPatternCross(stateCount);
}

/**
 * <p>After a horizontal scan finds a potential finder pattern, this method
 * "cross-checks" by scanning down vertically through the center of the possible
 * finder pattern to see if the same proportion is detected.</p>
 *
 * @param image that is checked for a barcode.
 * @param startI   row where a finder pattern was detected
 * @param centerJ  center of the section that appears to cross a finder pattern
 * @param maxCount maximum reasonable number of modules that should be
 *                 observed in any reading state, based on the results of the horizontal scan
 * @return vertical center of finder pattern, or {@link Float#NaN} if not found
 */
float FinderPatternFinder::crossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount,
											  int originalStateCountTotal)
{
	int maxI = image.height();
	std::vector<int> stateCount = getCrossCheckStateCount();

	// Start counting up from center
	int i = startI;
	while (i >= 0 && image.get(centerJ, i)) {
		stateCount[2]++;
		i--;
	}
	if (i < 0) {
		return NAN;
	}
	while (i >= 0 && !image.get(centerJ, i) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		i--;
	}
	// If already too many modules in this state or ran off the edge:
	if (i < 0 || stateCount[1] > maxCount) {
		return NAN;
	}
	while (i >= 0 && image.get(centerJ, i) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		i--;
	}
	if (stateCount[0] > maxCount) {
		return NAN;
	}

	// Now also count down from center
	i = startI + 1;
	while (i < maxI && image.get(centerJ, i)) {
		stateCount[2]++;
		i++;
	}
	if (i == maxI) {
		return NAN;
	}
	while (i < maxI && !image.get(centerJ, i) && stateCount[3] < maxCount) {
		stateCount[3]++;
		i++;
	}
	if (i == maxI || stateCount[3] >= maxCount) {
		return NAN;
	}
	while (i < maxI && image.get(centerJ, i) && stateCount[4] < maxCount) {
		stateCount[4]++;
		i++;
	}
	if (stateCount[4] >= maxCount) {
		return NAN;
	}

	// If we found a finder-pattern-like section, but its size is more than 40% different than
	// the original, assume it's a false positive
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
		return NAN;
	}

	return foundPatternCross(stateCount) ? centerFromEnd(stateCount, i) : NAN;
}

/**
 * <p>Like {@link #crossCheckVertical(int, int, int, int)}, and in fact is basically identical,
 * except it reads horizontally instead of vertically. This is used to cross-cross
 * check a vertical cross check and locate the real center of the alignment pattern.</p>
 */
float FinderPatternFinder::crossCheckHorizontal(const BitMatrix& image, int startJ, int centerI, int maxCount,
												int originalStateCountTotal)
{
	int maxJ = image.width();
	std::vector<int> stateCount = getCrossCheckStateCount();

	int j = startJ;
	while (j >= 0 && image.get(j, centerI)) {
		stateCount[2]++;
		j--;
	}
	if (j < 0) {
		return NAN;
	}
	while (j >= 0 && !image.get(j, centerI) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		j--;
	}
	if (j < 0 || stateCount[1] > maxCount) {
		return NAN;
	}
	while (j >= 0 && image.get(j, centerI) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		j--;
	}
	if (stateCount[0] > maxCount) {
		return NAN;
	}

	j = startJ + 1;
	while (j < maxJ && image.get(j, centerI)) {
		stateCount[2]++;
		j++;
	}
	if (j == maxJ) {
		return NAN;
	}
	while (j < maxJ && !image.get(j, centerI) && stateCount[3] < maxCount) {
		stateCount[3]++;
		j++;
	}
	if (j == maxJ || stateCount[3] >= maxCount) {
		return NAN;
	}
	while (j < maxJ && image.get(j, centerI) && stateCount[4] < maxCount) {
		stateCount[4]++;
		j++;
	}
	if (stateCount[4] >= maxCount) {
		return NAN;
	}

	// If we found a finder-pattern-like section, but its size is significantly different than
	// the original, assume it's a false positive
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	if (5 * abs(stateCountTotal - originalStateCountTotal) >= originalStateCountTotal) {
		return NAN;
	}

	return foundPatternCross(stateCount) ? centerFromEnd(stateCount, j) : NAN;
}

/**
 * <p>This is called when a horizontal scan finds a possible alignment pattern. It will
 * cross check with a vertical scan, and if successful, will, ah, cross-cross-check
 * with another horizontal scan. This is needed primarily to locate the real horizontal
 * center of the pattern in cases of extreme skew.
 * And then we cross-cross-cross check with another diagonal scan.</p>
 * <p>
 * <p>If that succeeds the finder pattern location is added to a list that tracks
 * the number of times each location has been nearly-matched as a finder pattern.
 * Each additional find is more evidence that the location is in fact a finder
 * pattern center
 *
 * @param image
 * @param stateCount  reading state module counts from horizontal scan
 * @param i           row where finder pattern may be found
 * @param j           end of possible finder pattern in row
 * @param pureBarcode true if in "pure barcode" mode
 * @return true if a finder pattern candidate was found this time
 */
bool FinderPatternFinder::handlePossibleCenter(const BitMatrix& image, const std::vector<int>& stateCount, int i, int j,
											   bool pureBarcode)
{
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	float centerJ = centerFromEnd(stateCount, j);
	float centerI = crossCheckVertical(image, i, std::lroundf(centerJ), stateCount[2], stateCountTotal);
	if (!std::isnan(centerI)) {
		// Re-cross check
		centerJ =
			crossCheckHorizontal(image, std::lroundf(centerJ), std::lroundf(centerI), stateCount[2], stateCountTotal);
		if (!std::isnan(centerJ) &&
			(!pureBarcode ||
			 crossCheckDiagonal(image, std::lroundf(centerI), std::lroundf(centerJ), stateCount[2], stateCountTotal))) {
			float estimatedModuleSize = (float)stateCountTotal / 7.0f;
			bool found = false;
			for (size_t index = 0; index < possibleCenters_.size(); index++) {
				FinderPattern center = possibleCenters_[index];
				// Look for about the same center and module size:
				if (center.aboutEquals(estimatedModuleSize, centerI, centerJ)) {
					possibleCenters_[index] = center.combineEstimate(centerI, centerJ, estimatedModuleSize);
					found = true;
					break;
				}
			}
			if (!found) {
				FinderPattern newPattern(centerJ, centerI, estimatedModuleSize);
				possibleCenters_.push_back(newPattern);
			}
			return true;
		}
	}
	return false;
}

/**
 * @return true if we have found a finder pattern that have been detected
 * at least {@link #CENTER_QUORUM} times.
 */
bool FinderPatternFinder::haveMultiplyConfirmedCenters() const
{
	for (const FinderPattern& pattern : possibleCenters_) {
		if (pattern.getCount() >= CENTER_QUORUM) {
			return true;
		}
	}

	return false;
}

/**
 * @return the best {@link FinderPattern} from our list of candidates. The "best" is
 * the one that have been detected at least {@link #CENTER_QUORUM} times, and whose module
 * size differs from the average among those patterns the least
 */
std::optional<FinderPattern> FinderPatternFinder::selectBestPattern()
{
	if (possibleCenters_.empty())
		return {};

	if (possibleCenters_.size() > 1) {
		// Throw away all but those first size candidate points we found.
		float totalModuleSize = 0.0f;
		for (const FinderPattern& possibleCenter : possibleCenters_) {
			totalModuleSize += possibleCenter.getEstimatedModuleSize();
		}

		float average = totalModuleSize / possibleCenters_.size();
		std::sort(possibleCenters_.begin(), possibleCenters_.end(), CenterComparator(average));

		possibleCenters_.erase(possibleCenters_.begin() + 1, possibleCenters_.end());
	}

	return possibleCenters_[0];
}

} // namespace ZXing::MicroQRCode
