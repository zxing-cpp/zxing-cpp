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

#include "aztec/AZEncoder.h"
#include "aztec/AZHighLevelEncoder.h"
#include "BitArray.h"
#include "ReedSolomonEncoder.h"
#include "GenericGF.h"
#include "ZXStrConvWorkaround.h"
#include "ZXTestSupport.h"

#include <cstdlib>

namespace ZXing {
namespace Aztec {

static const int MAX_NB_BITS = 32;
static const int MAX_NB_BITS_COMPACT = 4;

static const int WORD_SIZE[] = {
	4, 6, 6, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12
};

static void DrawBullsEye(BitMatrix& matrix, int center, int size)
{
	for (int i = 0; i < size; i += 2) {
		for (int j = center - i; j <= center + i; j++) {
			matrix.set(j, center - i);
			matrix.set(j, center + i);
			matrix.set(center - i, j);
			matrix.set(center + i, j);
		}
	}
	matrix.set(center - size, center - size);
	matrix.set(center - size + 1, center - size);
	matrix.set(center - size, center - size + 1);
	matrix.set(center + size, center - size);
	matrix.set(center + size, center - size + 1);
	matrix.set(center + size, center + size - 1);
}


static const GenericGF& GetGFFromWordSize(int wordSize)
{
	switch (wordSize) {
	case 4:
		return GenericGF::AztecParam();
	case 6:
		return GenericGF::AztecData6();
	case 8:
		return GenericGF::AztecData8();
	case 10:
		return GenericGF::AztecData10();
	case 12:
		return GenericGF::AztecData12();
	default:
		throw std::invalid_argument("Unsupported word size " + std::to_string(wordSize));
	}
}

static std::vector<int> BitsToWords(const BitArray& stuffedBits, int wordSize, int totalWords)
{
	std::vector<int> message(totalWords, 0);
	int i;
	int n;
	for (i = 0, n = stuffedBits.size() / wordSize; i < n; i++) {
		int value = 0;
		for (int j = 0; j < wordSize; j++) {
			value |= stuffedBits.get(i * wordSize + j) ? (1 << (wordSize - j - 1)) : 0;
		}
		message[i] = value;
	}
	return message;
}

static void GenerateCheckWords(const BitArray& bitArray, int totalBits, int wordSize, BitArray& messageBits)
{
	// bitArray is guaranteed to be a multiple of the wordSize, so no padding needed
	int messageSizeInWords = bitArray.size() / wordSize;
	ReedSolomonEncoder rs(GetGFFromWordSize(wordSize));
	int totalWords = totalBits / wordSize;
	std::vector<int> messageWords = BitsToWords(bitArray, wordSize, totalWords);
	rs.encode(messageWords, totalWords - messageSizeInWords);
	int startPad = totalBits % wordSize;
	messageBits = BitArray();
	messageBits.appendBits(0, startPad);
	for (int messageWord : messageWords) {
		messageBits.appendBits(messageWord, wordSize);
	}
}

ZXING_EXPORT_TEST_ONLY
void GenerateModeMessage(bool compact, int layers, int messageSizeInWords, BitArray& modeMessage)
{
	modeMessage = BitArray();
	if (compact) {
		modeMessage.appendBits(layers - 1, 2);
		modeMessage.appendBits(messageSizeInWords - 1, 6);
		GenerateCheckWords(modeMessage, 28, 4, modeMessage);
	}
	else {
		modeMessage.appendBits(layers - 1, 5);
		modeMessage.appendBits(messageSizeInWords - 1, 11);
		GenerateCheckWords(modeMessage, 40, 4, modeMessage);
	}
}

static void DrawModeMessage(BitMatrix& matrix, bool compact, int matrixSize, const BitArray& modeMessage)
{
	int center = matrixSize / 2;
	if (compact) {
		for (int i = 0; i < 7; i++) {
			int offset = center - 3 + i;
			if (modeMessage.get(i)) {
				matrix.set(offset, center - 5);
			}
			if (modeMessage.get(i + 7)) {
				matrix.set(center + 5, offset);
			}
			if (modeMessage.get(20 - i)) {
				matrix.set(offset, center + 5);
			}
			if (modeMessage.get(27 - i)) {
				matrix.set(center - 5, offset);
			}
		}
	}
	else {
		for (int i = 0; i < 10; i++) {
			int offset = center - 5 + i + i / 5;
			if (modeMessage.get(i)) {
				matrix.set(offset, center - 7);
			}
			if (modeMessage.get(i + 10)) {
				matrix.set(center + 7, offset);
			}
			if (modeMessage.get(29 - i)) {
				matrix.set(offset, center + 7);
			}
			if (modeMessage.get(39 - i)) {
				matrix.set(center - 7, offset);
			}
		}
	}
}

ZXING_EXPORT_TEST_ONLY
void StuffBits(const BitArray& bits, int wordSize, BitArray& out)
{
	out = BitArray();
	int n = bits.size();
	int mask = (1 << wordSize) - 2;
	for (int i = 0; i < n; i += wordSize) {
		int word = 0;
		for (int j = 0; j < wordSize; j++) {
			if (i + j >= n || bits.get(i + j)) {
				word |= 1 << (wordSize - 1 - j);
			}
		}
		if ((word & mask) == mask) {
			out.appendBits(word & mask, wordSize);
			i--;
		}
		else if ((word & mask) == 0) {
			out.appendBits(word | 1, wordSize);
			i--;
		}
		else {
			out.appendBits(word, wordSize);
		}
	}
}

static int TotalBitsInLayer(int layers, bool compact)
{
	return ((compact ? 88 : 112) + 16 * layers) * layers;
}

/**
* Encodes the given binary content as an Aztec symbol
*
* @param data input data string
* @param minECCPercent minimal percentage of error check words (According to ISO/IEC 24778:2008,
*                      a minimum of 23% + 3 words is recommended)
* @param userSpecifiedLayers if non-zero, a user-specified value for the number of layers
* @return Aztec symbol matrix with metadata
*/
EncodeResult
Encoder::Encode(const std::string& data, int minECCPercent, int userSpecifiedLayers)
{
	// High-level encode
	BitArray bits = HighLevelEncoder::Encode(data);

	// stuff bits and choose symbol size
	int eccBits = bits.size() * minECCPercent / 100 + 11;
	int totalSizeBits = bits.size() + eccBits;
	bool compact;
	int layers;
	int totalBitsInLayer;
	int wordSize;
	BitArray stuffedBits;
	if (userSpecifiedLayers != DEFAULT_AZTEC_LAYERS) {
		compact = userSpecifiedLayers < 0;
		layers = std::abs(userSpecifiedLayers);
		if (layers > (compact ? MAX_NB_BITS_COMPACT : MAX_NB_BITS)) {
			throw std::invalid_argument("Illegal value for layers: " + std::to_string(userSpecifiedLayers));
		}
		totalBitsInLayer = TotalBitsInLayer(layers, compact);
		wordSize = WORD_SIZE[layers];
		int usableBitsInLayers = totalBitsInLayer - (totalBitsInLayer % wordSize);
		StuffBits(bits, wordSize, stuffedBits);
		if (stuffedBits.size() + eccBits > usableBitsInLayers) {
			throw std::invalid_argument("Data to large for user specified layer");
		}
		if (compact && stuffedBits.size() > wordSize * 64) {
			// Compact format only allows 64 data words, though C4 can hold more words than that
			throw std::invalid_argument("Data to large for user specified layer");
		}
	}
	else {
		wordSize = 0;
		// We look at the possible table sizes in the order Compact1, Compact2, Compact3,
		// Compact4, Normal4,...  Normal(i) for i < 4 isn't typically used since Compact(i+1)
		// is the same size, but has more data.
		for (int i = 0; ; i++) {
			if (i > MAX_NB_BITS) {
				throw std::invalid_argument("Data too large for an Aztec code");
			}
			compact = i <= 3;
			layers = compact ? i + 1 : i;
			totalBitsInLayer = TotalBitsInLayer(layers, compact);
			if (totalSizeBits > totalBitsInLayer) {
				continue;
			}
			// [Re]stuff the bits if this is the first opportunity, or if the
			// wordSize has changed
			if (wordSize != WORD_SIZE[layers]) {
				wordSize = WORD_SIZE[layers];
				StuffBits(bits, wordSize, stuffedBits);
			}
			int usableBitsInLayers = totalBitsInLayer - (totalBitsInLayer % wordSize);
			if (compact && stuffedBits.size() > wordSize * 64) {
				// Compact format only allows 64 data words, though C4 can hold more words than that
				continue;
			}
			if (stuffedBits.size() + eccBits <= usableBitsInLayers) {
				break;
			}
		}
	}
	BitArray messageBits;
	GenerateCheckWords(stuffedBits, totalBitsInLayer, wordSize, messageBits);

	// generate mode message
	int messageSizeInWords = stuffedBits.size() / wordSize;
	BitArray modeMessage;
	GenerateModeMessage(compact, layers, messageSizeInWords, modeMessage);

	// allocate symbol
	int baseMatrixSize = (compact ? 11 : 14) + layers * 4; // not including alignment lines
	std::vector<int> alignmentMap(baseMatrixSize, 0);
	int matrixSize;
	if (compact) {
		// no alignment marks in compact mode, alignmentMap is a no-op
		matrixSize = baseMatrixSize;
		for (int i = 0; i < (int)alignmentMap.size(); i++) {
			alignmentMap[i] = i;
		}
	}
	else {
		matrixSize = baseMatrixSize + 1 + 2 * ((baseMatrixSize / 2 - 1) / 15);
		int origCenter = baseMatrixSize / 2;
		int center = matrixSize / 2;
		for (int i = 0; i < origCenter; i++) {
			int newOffset = i + i / 15;
			alignmentMap[origCenter - i - 1] = center - newOffset - 1;
			alignmentMap[origCenter + i] = center + newOffset + 1;
		}
	}

	EncodeResult output{compact, matrixSize, layers, messageSizeInWords, BitMatrix(matrixSize)};

	BitMatrix& matrix = output.matrix;

	// draw data bits
	for (int i = 0, rowOffset = 0; i < layers; i++) {
		int rowSize = (layers - i) * 4 + (compact ? 9 : 12);
		for (int j = 0; j < rowSize; j++) {
			int columnOffset = j * 2;
			for (int k = 0; k < 2; k++) {
				if (messageBits.get(rowOffset + columnOffset + k)) {
					matrix.set(alignmentMap[i * 2 + k], alignmentMap[i * 2 + j]);
				}
				if (messageBits.get(rowOffset + rowSize * 2 + columnOffset + k)) {
					matrix.set(alignmentMap[i * 2 + j], alignmentMap[baseMatrixSize - 1 - i * 2 - k]);
				}
				if (messageBits.get(rowOffset + rowSize * 4 + columnOffset + k)) {
					matrix.set(alignmentMap[baseMatrixSize - 1 - i * 2 - k], alignmentMap[baseMatrixSize - 1 - i * 2 - j]);
				}
				if (messageBits.get(rowOffset + rowSize * 6 + columnOffset + k)) {
					matrix.set(alignmentMap[baseMatrixSize - 1 - i * 2 - j], alignmentMap[i * 2 + k]);
				}
			}
		}
		rowOffset += rowSize * 8;
	}

	// draw mode message
	DrawModeMessage(matrix, compact, matrixSize, modeMessage);

	// draw alignment marks
	if (compact) {
		DrawBullsEye(matrix, matrixSize / 2, 5);
	}
	else {
		DrawBullsEye(matrix, matrixSize / 2, 7);
		for (int i = 0, j = 0; i < baseMatrixSize / 2 - 1; i += 15, j += 16) {
			for (int k = (matrixSize / 2) & 1; k < matrixSize; k += 2) {
				matrix.set(matrixSize / 2 - j, k);
				matrix.set(matrixSize / 2 + j, k);
				matrix.set(k, matrixSize / 2 - j);
				matrix.set(k, matrixSize / 2 + j);
			}
		}
	}
	return output;
}

} // Aztec
} // ZXing
