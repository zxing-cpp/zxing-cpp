/*
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

#include "qrcode/QRAlignmentPatternFinder.h"
#include "qrcode/QRAlignmentPattern.h"
#include "BitMatrix.h"

#include <array>

namespace ZXing {
namespace QRCode {


typedef std::array<int, 3> StateCount;

	//private final BitMatrix image;
	//private final List<AlignmentPattern> possibleCenters;
	//private final int startX;
	//private final int startY;
	//private final int width;
	//private final int height;
	//private final float moduleSize;
	//private final int[] crossCheckStateCount;
	//private final ResultPointCallback resultPointCallback;

	///**
	//* <p>Creates a finder that will look in a portion of the whole image.</p>
	//*
	//* @param image image to search
	//* @param startX left column from which to start searching
	//* @param startY top row from which to start searching
	//* @param width width of region to search
	//* @param height height of region to search
	//* @param moduleSize estimated module size so far
	//*/
	//AlignmentPatternFinder(BitMatrix image,
	//	int startX,
	//	int startY,
	//	int width,
	//	int height,
	//	float moduleSize,
	//	ResultPointCallback resultPointCallback) {
	//	this.image = image;
	//	this.possibleCenters = new ArrayList<>(5);
	//	this.startX = startX;
	//	this.startY = startY;
	//	this.width = width;
	//	this.height = height;
	//	this.moduleSize = moduleSize;
	//	this.crossCheckStateCount = new int[3];
	//	this.resultPointCallback = resultPointCallback;
	//}

/**
* Given a count of black/white/black pixels just seen and an end position,
* figures the location of the center of this black/white/black run.
*/
static float CenterFromEnd(const StateCount& stateCount, int end)
{
	return static_cast<float>(end - stateCount[2]) - stateCount[1] / 2.0f;
}

/**
* @param stateCount count of black/white/black pixels just read
* @return true iff the proportions of the counts is close enough to the 1/1/1 ratios
*         used by alignment patterns to be considered a match
*/
static bool FoundPatternCross(const StateCount& stateCount, float moduleSize)
{
	float maxVariance = moduleSize / 2.0f;
	for (int i = 0; i < 3; i++) {
		if (std::abs(moduleSize - stateCount[i]) >= maxVariance) {
			return false;
		}
	}
	return true;
}

/**
* <p>After a horizontal scan finds a potential alignment pattern, this method
* "cross-checks" by scanning down vertically through the center of the possible
* alignment pattern to see if the same proportion is detected.</p>
*
* @param startI row where an alignment pattern was detected
* @param centerJ center of the section that appears to cross an alignment pattern
* @param maxCount maximum reasonable number of modules that should be
* observed in any reading state, based on the results of the horizontal scan
* @return vertical center of alignment pattern, or {@link Float#NaN} if not found
*/
static float CrossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount, int originalStateCountTotal, float moduleSize)
{
	int maxI = image.height();
	StateCount stateCount = { 0, 0, 0 };

	// Start counting up from center
	int i = startI;
	while (i >= 0 && image.get(centerJ, i) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		i--;
	}
	// If already too many modules in this state or ran off the edge:
	if (i < 0 || stateCount[1] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i >= 0 && !image.get(centerJ, i) && stateCount[0] <= maxCount) {
		stateCount[0]++;
		i--;
	}
	if (stateCount[0] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	// Now also count down from center
	i = startI + 1;
	while (i < maxI && image.get(centerJ, i) && stateCount[1] <= maxCount) {
		stateCount[1]++;
		i++;
	}
	if (i == maxI || stateCount[1] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}
	while (i < maxI && !image.get(centerJ, i) && stateCount[2] <= maxCount) {
		stateCount[2]++;
		i++;
	}
	if (stateCount[2] > maxCount) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
	if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
		return std::numeric_limits<float>::quiet_NaN();
	}

	return FoundPatternCross(stateCount, moduleSize) ? CenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN();
}

/**
* <p>This is called when a horizontal scan finds a possible alignment pattern. It will
* cross check with a vertical scan, and if successful, will see if this pattern had been
* found on a previous horizontal scan. If so, we consider it confirmed and conclude we have
* found the alignment pattern.</p>
*
* @param stateCount reading state module counts from horizontal scan
* @param i row where alignment pattern may be found
* @param j end of possible alignment pattern in row
* @return {@link AlignmentPattern} if we have found the same pattern twice, or null if not
*/
static bool HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j, float moduleSize, const ResultPointCallback& resultPointCallback, AlignmentPattern& confirm, std::vector<AlignmentPattern>& possibleCenters)
{
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2];
	float centerJ = CenterFromEnd(stateCount, j);
	float centerI = CrossCheckVertical(image, i, static_cast<int>(centerJ), 2 * stateCount[1], stateCountTotal, moduleSize);
	if (!std::isnan(centerI)) {
		float estimatedModuleSize = (float)(stateCount[0] + stateCount[1] + stateCount[2]) / 3.0f;
		for (const AlignmentPattern& center : possibleCenters) {
			// Look for about the same center and module size:
			if (center.aboutEquals(estimatedModuleSize, centerI, centerJ)) {
				confirm = center.combineEstimate(centerI, centerJ, estimatedModuleSize);
				return true;
			}
		}
		// Hadn't found this before; save it
		possibleCenters.emplace_back(centerJ, centerI, estimatedModuleSize);
		if (resultPointCallback != nullptr) {
			resultPointCallback(possibleCenters.back());
		}
	}
	return false;
}

bool
AlignmentPatternFinder::Find(const BitMatrix& image, int startX, int startY, int width, int height, float moduleSize, const ResultPointCallback& resultPointCallback, AlignmentPattern &result)
{
	int maxJ = startX + width;
	int middleI = startY + (height / 2);
	std::vector<AlignmentPattern> possibleCenters;
	possibleCenters.resize(5);

	// We are looking for black/white/black modules in 1:1:1 ratio;
	// this tracks the number of black/white/black modules seen so far
	for (int iGen = 0; iGen < height; iGen++) {
		// Search from middle outwards
		StateCount stateCount = { 0, 0, 0 };
		int i = middleI + ((iGen & 0x01) == 0 ? (iGen + 1) / 2 : -((iGen + 1) / 2));
		int j = startX;
		// Burn off leading white pixels before anything else; if we start in the middle of
		// a white run, it doesn't make sense to count its length, since we don't know if the
		// white run continued to the left of the start point
		while (j < maxJ && !image.get(j, i)) {
			j++;
		}
		int currentState = 0;
		while (j < maxJ) {
			if (image.get(j, i)) {
				// Black pixel
				if (currentState == 1) { // Counting black pixels
					stateCount[1]++;
				}
				else { // Counting white pixels
					if (currentState == 2) { // A winner?
						if (FoundPatternCross(stateCount, moduleSize)) { // Yes
							if (HandlePossibleCenter(image, stateCount, i, j, moduleSize, resultPointCallback, result, possibleCenters)) {
								return true;
							}
						}
						stateCount[0] = stateCount[2];
						stateCount[1] = 1;
						stateCount[2] = 0;
						currentState = 1;
					}
					else {
						stateCount[++currentState]++;
					}
				}
			}
			else { // White pixel
				if (currentState == 1) { // Counting black pixels
					currentState++;
				}
				stateCount[currentState]++;
			}
			j++;
		}
		if (FoundPatternCross(stateCount, moduleSize)) {
			if (HandlePossibleCenter(image, stateCount, i, maxJ, moduleSize, resultPointCallback, result, possibleCenters)) {
				return true;
			}
		}

	}

	// Hmm, nothing we saw was observed and confirmed twice. If we had
	// any guess at all, return it.
	if (!possibleCenters.empty()) {
		result = possibleCenters.front();
		return true;
	}

	return false;
}

} // QRCode
} // ZXing
