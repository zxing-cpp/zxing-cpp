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

#include "aztec/AZDecoder.h"
#include "aztec/AZDetectorResult.h"
#include "DecoderResult.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "DecodeStatus.h"
#include "BitMatrix.h"
#include "TextDecoder.h"
#include "ZXTestSupport.h"

#include <numeric>
#include <cstring>

namespace ZXing {
namespace Aztec {

enum class Table {
	UPPER,
	LOWER,
	MIXED,
	DIGIT,
	PUNCT,
	BINARY
};

static const char* UPPER_TABLE[] = {
	"CTRL_PS", " ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
	"Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "CTRL_LL", "CTRL_ML", "CTRL_DL", "CTRL_BS"
};

static const char* LOWER_TABLE[] = {
	"CTRL_PS", " ", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
	"q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "CTRL_US", "CTRL_ML", "CTRL_DL", "CTRL_BS"
};

static const char* MIXED_TABLE[] = {
	"CTRL_PS", " ", "\1", "\2", "\3", "\4", "\5", "\6", "\7", "\b", "\t", "\n",
	"\13", "\f", "\r", "\33", "\34", "\35", "\36", "\37", "@", "\\", "^", "_",
	"`", "|", "~", "\177", "CTRL_LL", "CTRL_UL", "CTRL_PL", "CTRL_BS"
};

static const char* PUNCT_TABLE[] = {
	"", "\r", "\r\n", ". ", ", ", ": ", "!", "\"", "#", "$", "%", "&", "'", "(", ")",
	"*", "+", ",", "-", ".", "/", ":", ";", "<", "=", ">", "?", "[", "]", "{", "}", "CTRL_UL"
};

static const char* DIGIT_TABLE[] = {
	"CTRL_PS", " ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ",", ".", "CTRL_UL", "CTRL_US"
};

inline static int TotalBitsInLayer(int layers, bool compact)
{
	return ((compact ? 88 : 112) + 16 * layers) * layers;
}

/**
* Gets the array of bits from an Aztec Code matrix
*
* @return the array of bits
*/
static std::vector<bool> ExtractBits(const DetectorResult& ddata)
{
	bool compact = ddata.isCompact();
	int layers = ddata.nbLayers();
	int baseMatrixSize = (compact ? 11 : 14) + layers * 4; // not including alignment lines
	std::vector<int> alignmentMap(baseMatrixSize, 0);

	if (compact) {
		std::iota(alignmentMap.begin(), alignmentMap.end(), 0);
	}
	else {
		int matrixSize = baseMatrixSize + 1 + 2 * ((baseMatrixSize / 2 - 1) / 15);
		int origCenter = baseMatrixSize / 2;
		int center = matrixSize / 2;
		for (int i = 0; i < origCenter; i++) {
			int newOffset = i + i / 15;
			alignmentMap[origCenter - i - 1] = center - newOffset - 1;
			alignmentMap[origCenter + i] = center + newOffset + 1;
		}
	}
	auto& matrix = ddata.bits();
	std::vector<bool> rawbits(TotalBitsInLayer(layers, compact));
	for (int i = 0, rowOffset = 0; i < layers; i++) {
		int rowSize = (layers - i) * 4 + (compact ? 9 : 12);
		// The top-left most point of this layer is <low, low> (not including alignment lines)
		int low = i * 2;
		// The bottom-right most point of this layer is <high, high> (not including alignment lines)
		int high = baseMatrixSize - 1 - low;
		// We pull bits from the two 2 x rowSize columns and two rowSize x 2 rows
		for (int j = 0; j < rowSize; j++) {
			int columnOffset = j * 2;
			for (int k = 0; k < 2; k++) {
				// left column
				rawbits[rowOffset + columnOffset + k] =
					matrix.get(alignmentMap[low + k], alignmentMap[low + j]);
				// bottom row
				rawbits[rowOffset + 2 * rowSize + columnOffset + k] =
					matrix.get(alignmentMap[low + j], alignmentMap[high - k]);
				// right column
				rawbits[rowOffset + 4 * rowSize + columnOffset + k] =
					matrix.get(alignmentMap[high - k], alignmentMap[high - j]);
				// top row
				rawbits[rowOffset + 6 * rowSize + columnOffset + k] =
					matrix.get(alignmentMap[high - j], alignmentMap[low + k]);
			}
		}
		rowOffset += rowSize * 8;
	}
	return rawbits;
}

/**
* Reads a code of given length and at given index in an array of bits
*/
static int ReadCode(const std::vector<bool>& rawbits, int startIndex, int length)
{
	int res = 0;
	for (int i = startIndex; i < startIndex + length; i++) {
		res = (res << 1) | static_cast<int>(rawbits[i]);
	}
	return res;
}

/**
* <p>Performs RS error correction on an array of bits.</p>
*
* @return the corrected array
* @throws FormatException if the input contains too many errors
*/
static bool CorrectBits(const DetectorResult& ddata, const std::vector<bool>& rawbits, std::vector<bool>& correctedBits)
{
	const GenericGF* gf = nullptr;
	int codewordSize;

	if (ddata.nbLayers() <= 2) {
		codewordSize = 6;
		gf = &GenericGF::AztecData6();
	}
	else if (ddata.nbLayers() <= 8) {
		codewordSize = 8;
		gf = &GenericGF::AztecData8();
	}
	else if (ddata.nbLayers() <= 22) {
		codewordSize = 10;
		gf = &GenericGF::AztecData10();
	}
	else {
		codewordSize = 12;
		gf = &GenericGF::AztecData12();
	}

	int numDataCodewords = ddata.nbDatablocks();
	int numCodewords = static_cast<int>(rawbits.size()) / codewordSize;
	if (numCodewords < numDataCodewords) {
		return false;
	}
	int offset = rawbits.size() % codewordSize;
	int numECCodewords = numCodewords - numDataCodewords;

	
	std::vector<int> dataWords(numCodewords);
	for (int i = 0; i < numCodewords; i++, offset += codewordSize) {
		dataWords[i] = ReadCode(rawbits, offset, codewordSize);
	}

	if (!ReedSolomonDecoder::Decode(*gf, dataWords, numECCodewords))
		return false;

	// Now perform the unstuffing operation.
	// First, count how many bits are going to be thrown out as stuffing
	int mask = (1 << codewordSize) - 1;
	int stuffedBits = 0;
	for (int i = 0; i < numDataCodewords; i++) {
		int dataWord = dataWords[i];
		if (dataWord == 0 || dataWord == mask) {
			return false;
		}
		else if (dataWord == 1 || dataWord == mask - 1) {
			stuffedBits++;
		}
	}
	// Now, actually unpack the bits and remove the stuffing
	correctedBits.resize(numDataCodewords * codewordSize - stuffedBits);
	int index = 0;
	for (int i = 0; i < numDataCodewords; i++) {
		int dataWord = dataWords[i];
		if (dataWord == 1 || dataWord == mask - 1) {
			// next codewordSize-1 bits are all zeros or all ones
			std::fill_n(correctedBits.begin() + index, codewordSize - 1, dataWord > 1);
			index += codewordSize - 1;
		}
		else {
			for (int bit = codewordSize - 1; bit >= 0; --bit) {
				correctedBits[index++] = (dataWord & (1 << bit)) != 0;
			}
		}
	}
	return true;
}

/**
* gets the table corresponding to the char passed
*/
static Table GetTable(char t)
{
	switch (t) {
	case 'L':
		return Table::LOWER;
	case 'P':
		return Table::PUNCT;
	case 'M':
		return Table::MIXED;
	case 'D':
		return Table::DIGIT;
	case 'B':
		return Table::BINARY;
	case 'U':
	default:
		return Table::UPPER;
	}
}

/**
* Gets the character (or string) corresponding to the passed code in the given table
*
* @param table the table used
* @param code the code of the character
*/
static const char* GetCharacter(Table table, int code)
{
	switch (table) {
	case Table::UPPER:
		return UPPER_TABLE[code];
	case Table::LOWER:
		return LOWER_TABLE[code];
	case Table::MIXED:
		return MIXED_TABLE[code];
	case Table::PUNCT:
		return PUNCT_TABLE[code];
	case Table::DIGIT:
		return DIGIT_TABLE[code];
	default:
		return nullptr;
		// Should not reach here.
		//throw new IllegalStateException("Bad table");
	}
}

/**
* Gets the string encoded in the aztec code bits
*
* @return the decoded string
*/
ZXING_EXPORT_TEST_ONLY
std::string GetEncodedData(const std::vector<bool>& correctedBits)
{
	int endIndex = static_cast<int>(correctedBits.size());
	Table latchTable = Table::UPPER; // table most recently latched to
	Table shiftTable = Table::UPPER; // table to use for the next read
	std::string result;
	result.reserve(20);
	int index = 0;
	while (index < endIndex) {
		if (shiftTable == Table::BINARY) {
			if (endIndex - index < 5) {
				break;
			}
			int length = ReadCode(correctedBits, index, 5);
			index += 5;
			if (length == 0) {
				if (endIndex - index < 11) {
					break;
				}
				length = ReadCode(correctedBits, index, 11) + 31;
				index += 11;
			}
			for (int charCount = 0; charCount < length; charCount++) {
				if (endIndex - index < 8) {
					index = endIndex;  // Force outer loop to exit
					break;
				}
				int code = ReadCode(correctedBits, index, 8);
				result.push_back((char)code);
				index += 8;
			}
			// Go back to whatever mode we had been in
			shiftTable = latchTable;
		}
		else {
			int size = shiftTable == Table::DIGIT ? 4 : 5;
			if (endIndex - index < size) {
				break;
			}
			int code = ReadCode(correctedBits, index, size);
			index += size;
			const char* str = GetCharacter(shiftTable, code);
			if (std::strncmp(str, "CTRL_", 5) == 0) {
				// Table changes
				// ISO/IEC 24778:2008 prescibes ending a shift sequence in the mode from which it was invoked.
				// That's including when that mode is a shift.
				// Our test case dlusbs.png for issue #642 exercises that.
				latchTable = shiftTable;  // Latch the current mode, so as to return to Upper after U/S B/S
				shiftTable = GetTable(str[5]);
				if (str[6] == 'L') {
					latchTable = shiftTable;
				}
			}
			else {
				result.append(str);
				// Go back to whatever mode we had been in
				shiftTable = latchTable;
			}
		}
	}
	return result;
}

/**
* Reads a code of length 8 in an array of bits, padding with zeros
*/
static uint8_t ReadByte(const std::vector<bool>& rawbits, int startIndex)
{
	int n = static_cast<int>(rawbits.size()) - startIndex;
	if (n >= 8) {
		return static_cast<uint8_t>(ReadCode(rawbits, startIndex, 8));
	}
	return static_cast<uint8_t>(ReadCode(rawbits, startIndex, n) << (8 - n));
}

/**
* Packs a bit array into bytes, most significant bit first
*/
static ByteArray ConvertBoolArrayToByteArray(const std::vector<bool>& boolArr)
{
	ByteArray byteArr(((int)boolArr.size() + 7) / 8);
	for (int i = 0; i < byteArr.length(); ++i) {
		byteArr[i] = ReadByte(boolArr, 8 * i);
	}
	return byteArr;
}

DecoderResult Decoder::Decode(const DetectorResult& detectorResult)
{
	std::vector<bool> rawbits = ExtractBits(detectorResult);
	std::vector<bool> correctedBits;
	if (CorrectBits(detectorResult, rawbits, correctedBits)) {
		return DecoderResult(ConvertBoolArrayToByteArray(correctedBits),
							 TextDecoder::FromLatin1(GetEncodedData(correctedBits)))
		        .setNumBits(static_cast<int>(correctedBits.size()));
	}
	else {
		return DecodeStatus::FormatError;
	}
}

} // Aztec
} // ZXing
