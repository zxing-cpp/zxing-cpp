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
#include "DecodeStatus.h"
#include "DecoderResult.h"
#include "GenericGF.h"
#include "ReedSolomonDecoder.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "ZXTestSupport.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <string>
#include <vector>

namespace ZXing::Aztec {

enum class Table
{
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
static BitArray ExtractBits(const DetectorResult& ddata)
{
	bool compact = ddata.isCompact();
	int layers = ddata.nbLayers();
	int baseMatrixSize = (compact ? 11 : 14) + layers * 4; // not including alignment lines
	std::vector<int> map(baseMatrixSize, 0);

	if (compact) {
		std::iota(map.begin(), map.end(), 0);
	} else {
		int matrixSize = baseMatrixSize + 1 + 2 * ((baseMatrixSize / 2 - 1) / 15);
		int origCenter = baseMatrixSize / 2;
		int center = matrixSize / 2;
		for (int i = 0; i < origCenter; i++) {
			int newOffset = i + i / 15;
			map[origCenter - i - 1] = center - newOffset - 1;
			map[origCenter + i] = center + newOffset + 1;
		}
	}
	auto& matrix = ddata.bits();
	BitArray rawbits(TotalBitsInLayer(layers, compact));
	for (int i = 0, rowOffset = 0; i < layers; i++) {
		int rowSize = (layers - i) * 4 + (compact ? 9 : 12);
		// The top-left most point of this layer is <low, low> (not including alignment lines)
		int low = i * 2;
		// The bottom-right most point of this layer is <high, high> (not including alignment lines)
		int high = baseMatrixSize - 1 - low;
		// We pull bits from the two 2 x rowSize columns and two rowSize x 2 rows
		for (int j = 0; j < rowSize; j++) {
			int colOffset = j * 2;
			for (int k = 0; k < 2; k++) {
				// left column
				rawbits.set(rowOffset + 0 * rowSize + colOffset + k, matrix.get(map[low + k], map[low + j]));
				// bottom row
				rawbits.set(rowOffset + 2 * rowSize + colOffset + k, matrix.get(map[low + j], map[high - k]));
				// right column
				rawbits.set(rowOffset + 4 * rowSize + colOffset + k, matrix.get(map[high - k], map[high - j]));
				// top row
				rawbits.set(rowOffset + 6 * rowSize + colOffset + k, matrix.get(map[high - j], map[low + k]));
			}
		}
		rowOffset += rowSize * 8;
	}
	return rawbits;
}

/**
* <p>Performs RS error correction on an array of bits.</p>
*
* @return the corrected array
* @throws FormatException if the input contains too many errors
*/
static BitArray CorrectBits(const DetectorResult& ddata, const BitArray& rawbits)
{
	const GenericGF* gf = nullptr;
	int codewordSize;

	if (ddata.nbLayers() <= 2) {
		codewordSize = 6;
		gf = &GenericGF::AztecData6();
	} else if (ddata.nbLayers() <= 8) {
		codewordSize = 8;
		gf = &GenericGF::AztecData8();
	} else if (ddata.nbLayers() <= 22) {
		codewordSize = 10;
		gf = &GenericGF::AztecData10();
	} else {
		codewordSize = 12;
		gf = &GenericGF::AztecData12();
	}

	int numCodewords = Size(rawbits) / codewordSize;
	int numDataCodewords = ddata.nbDatablocks();
	int numECCodewords = numCodewords - numDataCodewords;

	if (numCodewords < numDataCodewords)
		return {};

	auto dataWords = ToInts<int>(rawbits, codewordSize, numCodewords, Size(rawbits) % codewordSize);

	if (!ReedSolomonDecode(*gf, dataWords, numECCodewords))
		return {};

	// drop the ECCodewords from the dataWords array
	dataWords.resize(numDataCodewords);

	// Now perform the unstuffing operation.
	BitArray correctedBits;
	// correctedBits.reserve(numDataCodewords * codewordSize - stuffedBits);
	for (int dataWord : dataWords) {
		if (dataWord == 0 || dataWord == (1 << codewordSize) - 1)
			return {};
		else if (dataWord == 1) // next codewordSize-1 bits are all zeros or all ones
			correctedBits.appendBits(0, codewordSize - 1);
		else if (dataWord == (1 << codewordSize) - 2)
			correctedBits.appendBits(0xffffffff, codewordSize - 1);
		else
			correctedBits.appendBits(dataWord, codewordSize);
	}

	return correctedBits;
}

/**
* gets the table corresponding to the char passed
*/
static Table GetTable(char t)
{
	switch (t) {
	case 'L': return Table::LOWER;
	case 'P': return Table::PUNCT;
	case 'M': return Table::MIXED;
	case 'D': return Table::DIGIT;
	case 'B': return Table::BINARY;
	case 'U':
	default: return Table::UPPER;
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
	case Table::UPPER: return UPPER_TABLE[code];
	case Table::LOWER: return LOWER_TABLE[code];
	case Table::MIXED: return MIXED_TABLE[code];
	case Table::PUNCT: return PUNCT_TABLE[code];
	case Table::DIGIT: return DIGIT_TABLE[code];
	case Table::BINARY: return nullptr; // should not happen
	}
	// silence gcc warning/error (this code can not be reached)
	return nullptr;
}

/**
* See ISO/IEC 24778:2008 Section 10.1
*/
static int ParseECIValue(BitArray::Range& bits, const int flg)
{
	int eci = 0;
	for (int i = 0; i < flg && bits.size() >= 4; i++)
		eci = 10 * eci + ReadBits(bits, 4) - 2;
	return eci;
}

/**
* See ISO/IEC 24778:2008 Section 8
*/
static StructuredAppendInfo ParseStructuredAppend(std::wstring& text)
{
	std::wstring id;
	std::string::size_type i = 0;

	if (text[0] == ' ') { // Space-delimited id
		std::string::size_type sp = text.find(' ', 1);
		if (sp == std::string::npos)
			return {};

		id = text.substr(1, sp - 1); // Strip space delimiters
		i = sp + 1;
	}
	if (i + 1 >= text.size() || !std::isupper(text[i]) || !std::isupper(text[i + 1]))
		return {};

	StructuredAppendInfo sai;
	sai.index = text[i] - 'A';
	sai.count = text[i + 1] - 'A' + 1;

	if (sai.count == 1 || sai.count <= sai.index) // If info doesn't make sense
		sai.count = 0; // Choose to mark count as unknown

	if (!id.empty())
		TextUtfEncoding::ToUtf8(id, sai.id);

	text.erase(0, i + 2); // Remove

	return sai;
}

struct AztecData
{
	std::wstring text;
	std::string symbologyIdentifier;
	StructuredAppendInfo sai;
};

/**
* Gets the string encoded in the aztec code bits
*
* @return the decoded string
*/
ZXING_EXPORT_TEST_ONLY
AztecData GetEncodedData(const BitArray& bits, const std::string& characterSet)
{
	AztecData res;
	Table latchTable = Table::UPPER; // table most recently latched to
	Table shiftTable = Table::UPPER; // table to use for the next read
	std::string text;
	text.reserve(20);
	int symbologyIdModifier = 0;
	CharacterSet encoding = CharacterSetECI::InitEncoding(characterSet);

	// Check for Structured Append - need 4 5-bit words, beginning with ML UL, ending with index and count
	bool haveStructuredAppend = Size(bits) > 20 && ToInt(bits, 0, 5) == 29 // ML (UPPER table)
								&& ToInt(bits, 5, 5) == 29; // UL (MIXED table)
	bool haveFNC1 = false;
	auto remBits = bits.range();

	while (remBits) {
		if (shiftTable == Table::BINARY) {
			if (remBits.size() < 5)
				break;
			int length = ReadBits(remBits, 5);
			if (length == 0) {
				if (remBits.size() < 11)
					break;
				length = ReadBits(remBits, 11) + 31;
			}
			for (int charCount = 0; charCount < length; charCount++) {
				if (remBits.size() < 8) {
					remBits.begin = remBits.end;  // Force outer loop to exit
					break;
				}
				int code = ReadBits(remBits, 8);
				text.push_back((char)code);
			}
			// Go back to whatever mode we had been in
			shiftTable = latchTable;
		} else {
			int size = shiftTable == Table::DIGIT ? 4 : 5;
			if (remBits.size() < size)
				break;
			int code = ReadBits(remBits, size);
			const char* str = GetCharacter(shiftTable, code);
			if (std::strncmp(str, "CTRL_", 5) == 0) {
				// Table changes
				// ISO/IEC 24778:2008 prescibes ending a shift sequence in the mode from which it was invoked.
				// That's including when that mode is a shift.
				// Our test case dlusbs.png for issue #642 exercises that.
				latchTable = shiftTable;  // Latch the current mode, so as to return to Upper after U/S B/S
				shiftTable = GetTable(str[5]);
				if (str[6] == 'L')
					latchTable = shiftTable;
			} else if (std::strcmp(str, "FLGN") == 0) {
				if (remBits.size() < 3)
					break;
				int flg = ReadBits(remBits, 3);
				if (flg == 0) { // FNC1
					haveFNC1 = true; // Will process first/second FNC1 at end after any Structured Append
					text.push_back((char)29); // May be removed at end if first/second FNC1
				} else if (flg <= 6) {
					// FLG(1) to FLG(6) ECI
					encoding =
						CharacterSetECI::OnChangeAppendReset(ParseECIValue(remBits, flg), res.text, text, encoding);
				} else {
					// FLG(7) is invalid
				}
				shiftTable = latchTable;
			} else {
				text.append(str);
				// Go back to whatever mode we had been in
				shiftTable = latchTable;
			}
		}
	}

	TextDecoder::Append(res.text, reinterpret_cast<const uint8_t*>(text.data()), text.size(), encoding);

	if (!res.text.empty()) {
		if (haveStructuredAppend)
			res.sai = ParseStructuredAppend(res.text);
		if (haveFNC1) {
			// As converting character set ECIs ourselves and ignoring/skipping non-character ECIs, not using
			// modifiers that indicate ECI protocol (ISO/IEC 24778:2008 Annex F Table F.1)
			if (res.text.front() == 29) {
				symbologyIdModifier = 1; // GS1
				res.text.erase(0, 1); // Remove FNC1
			} else if (res.text.size() > 2 && std::isupper(res.text[0]) && res.text[1] == 29) {
				// FNC1 following single uppercase letter (the AIM Application Indicator)
				symbologyIdModifier = 2; // AIM
				res.text.erase(1, 1); // Remove FNC1
				// The AIM Application Indicator character "A"-"Z" is left in the stream (ISO/IEC 24778:2008 16.2)
			} else if (res.text.size() > 3 && std::isdigit(res.text[0]) && std::isdigit(res.text[1]) &&
					   res.text[2] == 29) {
				// FNC1 following 2 digits (the AIM Application Indicator)
				symbologyIdModifier = 2; // AIM
				res.text.erase(2, 1); // Remove FNC1
				// The AIM Application Indicator characters "00"-"99" are left in the stream (ISO/IEC 24778:2008 16.2)
			}
		}
	}

	res.symbologyIdentifier = "]z" + std::to_string(symbologyIdModifier + (res.sai.index > -1 ? 6 : 0));

	return res;
}

DecoderResult Decode(const DetectorResult& detectorResult, const std::string& characterSet)
{
	BitArray bits = CorrectBits(detectorResult, ExtractBits(detectorResult));

	if (!bits.size())
		return DecodeStatus::FormatError;

	auto data = GetEncodedData(bits, characterSet);
	if (data.text.empty())
		return DecodeStatus::FormatError;

	return DecoderResult(bits.toBytes(), std::move(data.text))
		.setNumBits(Size(bits))
		.setSymbologyIdentifier(std::move(data.symbologyIdentifier))
		.setStructuredAppend(data.sai)
		.setReaderInit(detectorResult.readerInit());
}

} // namespace ZXing::Aztec
