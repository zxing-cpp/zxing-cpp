/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "qrcode/QRMaskUtil.h"
#include "qrcode/QRByteMatrix.h"

#include <algorithm>
#include <cstdlib>

namespace ZXing {
namespace QRCode {

// Penalty weights from section 6.8.2.1
static const int N1 = 3;
static const int N2 = 3;
static const int N3 = 40;
static const int N4 = 10;

/**
* Helper function for applyMaskPenaltyRule1. We need this for doing this calculation in both
* vertical and horizontal orders respectively.
*/
static int ApplyMaskPenaltyRule1Internal(const ByteMatrix& matrix, bool isHorizontal) {
	int penalty = 0;
	int width = matrix.width();
	int height = matrix.height();
	int iLimit = isHorizontal ? height : width;
	int jLimit = isHorizontal ? width : height;
	const int8_t* arr = matrix.getArray();
	for (int i = 0; i < iLimit; i++) {
		int numSameBitCells = 0;
		int prevBit = -1;
		for (int j = 0; j < jLimit; j++) {
			int bit = isHorizontal ? arr[i*width+j] : arr[j*width+i];
			if (bit == prevBit) {
				numSameBitCells++;
			}
			else {
				if (numSameBitCells >= 5) {
					penalty += N1 + (numSameBitCells - 5);
				}
				numSameBitCells = 1;  // Include the cell itself.
				prevBit = bit;
			}
		}
		if (numSameBitCells >= 5) {
			penalty += N1 + (numSameBitCells - 5);
		}
	}
	return penalty;
}


/**
* Apply mask penalty rule 1 and return the penalty. Find repetitive cells with the same color and
* give penalty to them. Example: 00000 or 11111.
*/
int MaskUtil::ApplyMaskPenaltyRule1(const ByteMatrix& matrix)
{
	return ApplyMaskPenaltyRule1Internal(matrix, true) + ApplyMaskPenaltyRule1Internal(matrix, false);
}

/**
* Apply mask penalty rule 2 and return the penalty. Find 2x2 blocks with the same color and give
* penalty to them. This is actually equivalent to the spec's rule, which is to find MxN blocks and give a
* penalty proportional to (M-1)x(N-1), because this is the number of 2x2 blocks inside such a block.
*/
int MaskUtil::ApplyMaskPenaltyRule2(const ByteMatrix& matrix)
{
	int penalty = 0;
	const int8_t* arr = matrix.getArray();
	int width = matrix.width();
	int height = matrix.height();
	for (int y = 0; y < height - 1; y++) {
		for (int x = 0; x < width - 1; x++) {
			int value = arr[y*width + x];
			if (value == arr[y*width + x + 1] && value == arr[(y + 1)*width + x] && value == arr[(y + 1)*width + x + 1]) {
				penalty++;
			}
		}
	}
	return N2 * penalty;
}

static bool IsWhiteHorizontal(const int8_t* rowArray, int from, int to)
{
	for (int i = from; i < to; i++) {
		if (rowArray[i] == 1) {
			return false;
		}
	}
	return true;
}

static bool IsWhiteVertical(const int8_t* array, int width, int col, int from, int to) {
	for (int i = from; i < to; i++) {
		if (array[i*width + col] == 1) {
			return false;
		}
	}
	return true;
}

/**
* Apply mask penalty rule 3 and return the penalty. Find consecutive runs of 1:1:3:1:1:4
* starting with black, or 4:1:1:3:1:1 starting with white, and give penalty to them.  If we
* find patterns like 000010111010000, we give penalty once.
*/
int MaskUtil::ApplyMaskPenaltyRule3(const ByteMatrix& matrix)
{
	int numPenalties = 0;
	const int8_t* arr = matrix.getArray();
	int width = matrix.width();
	int height = matrix.height();
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const int8_t* arrayY = arr + y*width;  // We can at least optimize this access
			if (x + 6 < width &&
				arrayY[x + 0] == 1 &&
				arrayY[x + 1] == 0 &&
				arrayY[x + 2] == 1 &&
				arrayY[x + 3] == 1 &&
				arrayY[x + 4] == 1 &&
				arrayY[x + 5] == 0 &&
				arrayY[x + 6] == 1 &&
				(IsWhiteHorizontal(arrayY, std::max(x - 4, 0), x)
					|| IsWhiteHorizontal(arrayY, x + 7, std::min(x + 11, width))))
			{
				numPenalties++;
			}
			if (y + 6 < height &&
				arr[(y + 0)*width + x] == 1 &&
				arr[(y + 1)*width + x] == 0 &&
				arr[(y + 2)*width + x] == 1 &&
				arr[(y + 3)*width + x] == 1 &&
				arr[(y + 4)*width + x] == 1 &&
				arr[(y + 5)*width + x] == 0 &&
				arr[(y + 6)*width + x] == 1 &&
				(IsWhiteVertical(arr, width, x, std::max(y - 4, 0), y)
					|| IsWhiteVertical(arr, width, x, y + 7, std::min(y + 11, height))))
			{
				numPenalties++;
			}
		}
	}
	return numPenalties * N3;
}

/**
* Apply mask penalty rule 4 and return the penalty. Calculate the ratio of dark cells and give
* penalty if the ratio is far from 50%. It gives 10 penalty for 5% distance.
*/
int MaskUtil::ApplyMaskPenaltyRule4(const ByteMatrix& matrix)
{
	int numDarkCells = 0;
	const int8_t* arr = matrix.getArray();
	int width = matrix.width();
	int height = matrix.height();
	for (int y = 0; y < height; y++) {
		const int8_t* arrayY = arr + y*width;
		for (int x = 0; x < width; x++) {
			if (arrayY[x] == 1) {
				numDarkCells++;
			}
		}
	}
	int numTotalCells = height * width;
	int fivePercentVariances = std::abs(numDarkCells * 2 - numTotalCells) * 10 / numTotalCells;
	return fivePercentVariances * N4;
}

} // QRCode
} // ZXing
