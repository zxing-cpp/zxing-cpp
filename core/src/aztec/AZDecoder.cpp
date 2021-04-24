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

#include "AZDecoder.h"

#include "AZDetectorResult.h"
#include "BitArray.h"
#include "BitMatrix.h"
#include "CharacterSetECI.h"
#include "DecoderResult.h"
#include "DecodeStatus.h"
#include "GenericGF.h"
#include "ReedSolomonDecoder.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "ZXTestSupport.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <string>
#include <vector>

namespace ZXing::Aztec {

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
	"FLGN", "\r", "\r\n", ". ", ", ", ": ", "!", "\"", "#", "$", "%", "&", "'", "(", ")",
	"*", "+", ",", "-", ".", "/", ":", ";", "<", "=", ">", "?", "[", "]", "{", "}", "CTRL_UL"
};

static const char* DIGIT_TABLE[] = {
	"CTRL_PS", " ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ",", ".", "CTRL_UL", "CTRL_US"
};

static int TotalBitsInLayer(int layers, bool compact)
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
	for (int i = startIndex; i < startIndex + length; i++)
		AppendBit(res, rawbits[i]);

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
	int numCodewords = Size(rawbits) / codewordSize;
	if (numCodewords < numDataCodewords) {
		return false;
	}
	int offset = rawbits.size() % codewordSize;
	int numECCodewords = numCodewords - numDataCodewords;

	
	std::vector<int> dataWords(numCodewords);
	for (int i = 0; i < numCodewords; i++, offset += codewordSize) {
		dataWords[i] = ReadCode(rawbits, offset, codewordSize);
	}

	if (!ReedSolomonDecode(*gf, dataWords, numECCodewords))
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
* See ISO/IEC 24778:2008 Section 10.1
*/
static int ParseECIValue(const std::vector<bool>& correctedBits, const int flg, const int endIndex, int& index)
{
	int eci = 0;
	for (int i = 0; i < flg && endIndex - index >= 4; i++) {
		eci *= 10;
		eci += ReadCode(correctedBits, index, 4) - 2;
		index += 4;
	}
	return eci;
}

/**
* See ISO/IEC 24778:2008 Section 8
*/
static void ParseStructuredAppend(std::wstring& resultEncoded, StructuredAppendInfo& sai)
{
	std::wstring id;
	int i = 0;

	if (resultEncoded[0] == ' ') { // Space-delimited id
		std::string::size_type sp = resultEncoded.find(' ', 1);
		if (sp == std::string::npos) {
			return;
		}
		id = resultEncoded.substr(1, sp - 1); // Strip space delimiters
		i = sp + 1;
	}
	if (i + 1 >= (int)resultEncoded.size() || resultEncoded[i] < 'A' || resultEncoded[i] > 'Z'
			|| resultEncoded[i + 1] < 'A' || resultEncoded[i + 1] > 'Z') {
		return;
	}
	sai.index = resultEncoded[i] - 'A';
	sai.count = resultEncoded[i + 1] - 'A' + 1;

	if (sai.count == 1 || sai.count <= sai.index) { // If info doesn't make sense
		sai.count = 0; // Choose to mark count as unknown
	}

	if (!id.empty()) {
		TextUtfEncoding::ToUtf8(id, sai.id);
	}

	resultEncoded.erase(0, i + 2); // Remove
}

/**
* Gets the string encoded in the aztec code bits
*
* @return the decoded string
*/
ZXING_EXPORT_TEST_ONLY
std::wstring GetEncodedData(const std::vector<bool>& correctedBits, const std::string& characterSet,
							StructuredAppendInfo& sai)
{
	int endIndex = Size(correctedBits);
	Table latchTable = Table::UPPER; // table most recently latched to
	Table shiftTable = Table::UPPER; // table to use for the next read
	std::string result;
	result.reserve(20);
	std::wstring resultEncoded;
	int index = 0;
	CharacterSet encoding = CharacterSetECI::InitEncoding(characterSet);

	// Check for Structured Append - need 4 5-bit words, beginning with ML UL, ending with index and count
	bool haveStructuredAppend = endIndex > 20 && ReadCode(correctedBits, 0, 5) == 29 // ML (UPPER table)
									&& ReadCode(correctedBits, 5, 5) == 29; // UL (MIXED table)

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
			else if (std::strcmp(str, "FLGN") == 0) {
				if (endIndex - index < 3) {
					break;
				}
				int flg = ReadCode(correctedBits, index, 3);
				index += 3;
				if (flg == 0) {
					// TODO: Handle FNC1
				}
				else if (flg <= 6) {
					// FLG(1) to FLG(6) ECI
					encoding = CharacterSetECI::OnChangeAppendReset(ParseECIValue(correctedBits, flg, endIndex, index),
																	resultEncoded, result, encoding);
				}
				else {
					// FLG(7) is invalid
				}
				shiftTable = latchTable;
			}
			else {
				result.append(str);
				// Go back to whatever mode we had been in
				shiftTable = latchTable;
			}
		}
	}
	TextDecoder::Append(resultEncoded, reinterpret_cast<const uint8_t*>(result.data()), result.size(), encoding);

	if (haveStructuredAppend && !resultEncoded.empty()) {
		ParseStructuredAppend(resultEncoded, sai);
	}

	return resultEncoded;
}

/**
* Reads a code of length 8 in an array of bits, padding with zeros
*/
static uint8_t ReadByte(const std::vector<bool>& rawbits, int startIndex)
{
	int n = Size(rawbits) - startIndex;
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
	ByteArray byteArr((Size(boolArr) + 7) / 8);
	for (int i = 0; i < Size(byteArr); ++i) {
		byteArr[i] = ReadByte(boolArr, 8 * i);
	}
	return byteArr;
}

DecoderResult Decoder::Decode(const DetectorResult& detectorResult, const std::string& characterSet)
{
	std::vector<bool> rawbits = ExtractBits(detectorResult);
	std::vector<bool> correctedBits;
	StructuredAppendInfo sai;
	if (CorrectBits(detectorResult, rawbits, correctedBits)) {
		return DecoderResult(ConvertBoolArrayToByteArray(correctedBits),
							 GetEncodedData(correctedBits, characterSet, sai))
		        .setNumBits(Size(correctedBits))
				.setStructuredAppend(sai)
				.setReaderInit(detectorResult.readerInit());
	}
	else {
		return DecodeStatus::FormatError;
	}
}

} // namespace ZXing::Aztec
