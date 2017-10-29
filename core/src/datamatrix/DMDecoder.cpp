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

#include "datamatrix/DMDecoder.h"
#include "datamatrix/DMBitMatrixParser.h"
#include "datamatrix/DMDataBlock.h"
#include "DecoderResult.h"
#include "BitMatrix.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "BitSource.h"
#include "DecodeStatus.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"
#include "ZXStrConvWorkaround.h"
#include "ZXTestSupport.h"

#include <array>

namespace ZXing {
namespace DataMatrix {

/**
* <p>Data Matrix Codes can encode text as bits in one of several modes, and can use multiple modes
* in one Data Matrix Code. This class decodes the bits back into text.</p>
*
* <p>See ISO 16022:2006, 5.2.1 - 5.2.9.2</p>
*
* @author bbrown@google.com (Brian Brown)
* @author Sean Owen
*/
namespace DecodedBitStreamParser {

enum Mode {
	FORMAT_ERROR,
	PAD_ENCODE, // Not really a mode
	ASCII_ENCODE,
	C40_ENCODE,
	TEXT_ENCODE,
	ANSIX12_ENCODE,
	EDIFACT_ENCODE,
	BASE256_ENCODE
};

/**
* See ISO 16022:2006, Annex C Table C.1
* The C40 Basic Character Set (*'s used for placeholders for the shift values)
*/
static const char C40_BASIC_SET_CHARS[] = {
	'*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

static const char C40_SHIFT2_SET_CHARS[] = {
	'!', '"', '#', '$', '%', '&', '\'', '(', ')', '*',  '+', ',', '-', '.',
	'/', ':', ';', '<', '=', '>', '?',  '@', '[', '\\', ']', '^', '_'
};

/**
* See ISO 16022:2006, Annex C Table C.2
* The Text Basic Character Set (*'s used for placeholders for the shift values)
*/
static const char TEXT_BASIC_SET_CHARS[] = {
	'*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

// Shift 2 for Text is the same encoding as C40
#define TEXT_SHIFT2_SET_CHARS C40_SHIFT2_SET_CHARS

static const char TEXT_SHIFT3_SET_CHARS[] = {
	'`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O',  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127
};

/**
* See ISO 16022:2006, 5.2.3 and Annex C, Table C.2
*/
static Mode DecodeAsciiSegment(BitSource& bits, std::string& result, std::string& resultTrailer)
{
	bool upperShift = false;
	do {
		int oneByte = bits.readBits(8);
		switch (oneByte) {
		case 0:
			return Mode::FORMAT_ERROR;
		case 129: // Pad
			return Mode::PAD_ENCODE;
		case 230: // Latch to C40 encodation
			return Mode::C40_ENCODE;
		case 231: // Latch to Base 256 encodation
			return Mode::BASE256_ENCODE;
		case 232: // FNC1
			result.push_back((char)29); // translate as ASCII 29
			break;
		case 233:
		case 234:
			// Structured Append, Reader Programming
			// Ignore these symbols for now
			//throw ReaderException.getInstance();
			break;
		case 235: // Upper Shift (shift to Extended ASCII)
			upperShift = true;
			break;
		case 236: // 05 Macro
			result.append("[)>\x1E""05\x1D");
			resultTrailer.insert(0, "\x1E\x04");
			break;
		case 237: // 06 Macro
			result.append("[)>\x1E""06\x1D");
			resultTrailer.insert(0, "\x1E\x04");
			break;
		case 238: // Latch to ANSI X12 encodation
			return Mode::ANSIX12_ENCODE;
		case 239: // Latch to Text encodation
			return Mode::TEXT_ENCODE;
		case 240: // Latch to EDIFACT encodation
			return Mode::EDIFACT_ENCODE;
		case 241:  // ECI Character
					// TODO(bbrown): I think we need to support ECI
					//throw ReaderException.getInstance();
					// Ignore this symbol for now
			break;
		default:
			if (oneByte <= 128) {  // ASCII data (ASCII value + 1)
				if (upperShift) {
					oneByte += 128;
					//upperShift = false;
				}
				result.push_back((char)(oneByte - 1));
				return Mode::ASCII_ENCODE;
			}
			else if (oneByte <= 229) {  // 2-digit data 00-99 (Numeric Value + 130)
				int value = oneByte - 130;
				if (value < 10) { // pad with '0' for single digit values
					result.push_back('0');
				}
				result.append(std::to_string(value));
			}
			else if (oneByte >= 242) {  // Not to be used in ASCII encodation
									// ... but work around encoders that end with 254, latch back to ASCII
				if (oneByte != 254 || bits.available() != 0) {
					return Mode::FORMAT_ERROR;
				}
			}
		}
	} while (bits.available() > 0);
	return Mode::ASCII_ENCODE;
}

static std::array<int, 3> ParseTwoBytes(int firstByte, int secondByte)
{
	int fullBitValue = (firstByte << 8) + secondByte - 1;
	int a = fullBitValue / 1600;
	fullBitValue -= a * 1600;
	int b = fullBitValue / 40;
	int c = fullBitValue - b * 40;
	return {a, b, c};
}

/**
* See ISO 16022:2006, 5.2.5 and Annex C, Table C.1
*/
static bool DecodeC40Segment(BitSource& bits, std::string& result)
{
	// Three C40 values are encoded in a 16-bit value as
	// (1600 * C1) + (40 * C2) + C3 + 1
	// TODO(bbrown): The Upper Shift with C40 doesn't work in the 4 value scenario all the time
	bool upperShift = false;

	int shift = 0;
	do {
		// If there is only one byte left then it will be encoded as ASCII
		if (bits.available() == 8) {
			return true;
		}
		int firstByte = bits.readBits(8);
		if (firstByte == 254) {  // Unlatch codeword
			return true;
		}

		for (int cValue : ParseTwoBytes(firstByte, bits.readBits(8))) {
			switch (shift) {
			case 0:
				if (cValue < 3) {
					shift = cValue + 1;
				}
				else if (cValue < Length(C40_BASIC_SET_CHARS)) {
					char c40char = C40_BASIC_SET_CHARS[cValue];
					if (upperShift) {
						result.push_back((char)(c40char + 128));
						upperShift = false;
					}
					else {
						result.push_back(c40char);
					}
				}
				else {
					return false;
				}
				break;
			case 1:
				if (upperShift) {
					result.push_back((char)(cValue + 128));
					upperShift = false;
				}
				else {
					result.push_back((char)cValue);
				}
				shift = 0;
				break;
			case 2:
				if (cValue < Length(C40_SHIFT2_SET_CHARS)) {
					char c40char = C40_SHIFT2_SET_CHARS[cValue];
					if (upperShift) {
						result.push_back((char)(c40char + 128));
						upperShift = false;
					}
					else {
						result.push_back(c40char);
					}
				}
				else if (cValue == 27) {  // FNC1
					result.push_back((char)29); // translate as ASCII 29
				}
				else if (cValue == 30) {  // Upper Shift
					upperShift = true;
				}
				else {
					return false;
				}
				shift = 0;
				break;
			case 3:
				if (upperShift) {
					result.push_back((char)(cValue + 224));
					upperShift = false;
				}
				else {
					result.push_back((char)(cValue + 96));
				}
				shift = 0;
				break;
			default:
				return false;
			}
		}
	} while (bits.available() > 0);
	return true;
}

/**
* See ISO 16022:2006, 5.2.6 and Annex C, Table C.2
*/
static bool DecodeTextSegment(BitSource& bits, std::string& result)
{
	// Three Text values are encoded in a 16-bit value as
	// (1600 * C1) + (40 * C2) + C3 + 1
	// TODO(bbrown): The Upper Shift with Text doesn't work in the 4 value scenario all the time
	bool upperShift = false;

	int shift = 0;
	do {
		// If there is only one byte left then it will be encoded as ASCII
		if (bits.available() == 8) {
			return true;
		}
		int firstByte = bits.readBits(8);
		if (firstByte == 254) {  // Unlatch codeword
			return true;
		}

		for (int cValue : ParseTwoBytes(firstByte, bits.readBits(8))) {
			switch (shift) {
			case 0:
				if (cValue < 3) {
					shift = cValue + 1;
				}
				else if (cValue < Length(TEXT_BASIC_SET_CHARS)) {
					char textChar = TEXT_BASIC_SET_CHARS[cValue];
					if (upperShift) {
						result.push_back((char)(textChar + 128));
						upperShift = false;
					}
					else {
						result.push_back(textChar);
					}
				}
				else {
					return false;
				}
				break;
			case 1:
				if (upperShift) {
					result.push_back((char)(cValue + 128));
					upperShift = false;
				}
				else {
					result.push_back((char)cValue);
				}
				shift = 0;
				break;
			case 2:
				// Shift 2 for Text is the same encoding as C40
				if (cValue < Length(TEXT_SHIFT2_SET_CHARS)) {
					char textChar = TEXT_SHIFT2_SET_CHARS[cValue];
					if (upperShift) {
						result.push_back((char)(textChar + 128));
						upperShift = false;
					}
					else {
						result.push_back(textChar);
					}
				}
				else if (cValue == 27) {  // FNC1
					result.push_back((char)29); // translate as ASCII 29
				}
				else if (cValue == 30) {  // Upper Shift
					upperShift = true;
				}
				else {
					return false;
				}
				shift = 0;
				break;
			case 3:
				if (cValue < Length(TEXT_SHIFT3_SET_CHARS)) {
					char textChar = TEXT_SHIFT3_SET_CHARS[cValue];
					if (upperShift) {
						result.push_back((char)(textChar + 128));
						upperShift = false;
					}
					else {
						result.push_back(textChar);
					}
					shift = 0;
				}
				else {
					return false;
				}
				break;
			default:
				return false;
			}
		}
	} while (bits.available() > 0);
	return true;
}

/**
* See ISO 16022:2006, 5.2.7
*/
static bool DecodeAnsiX12Segment(BitSource& bits, std::string& result)
{
	// Three ANSI X12 values are encoded in a 16-bit value as
	// (1600 * C1) + (40 * C2) + C3 + 1

	do {
		// If there is only one byte left then it will be encoded as ASCII
		if (bits.available() == 8) {
			return true;
		}
		int firstByte = bits.readBits(8);
		if (firstByte == 254) {  // Unlatch codeword
			return true;
		}

		for (auto cValue : ParseTwoBytes(firstByte, bits.readBits(8))) {
			if (cValue == 0) {  // X12 segment terminator <CR>
				result.push_back('\r');
			}
			else if (cValue == 1) {  // X12 segment separator *
				result.push_back('*');
			}
			else if (cValue == 2) {  // X12 sub-element separator >
				result.push_back('>');
			}
			else if (cValue == 3) {  // space
				result.push_back(' ');
			}
			else if (cValue < 14) {  // 0 - 9
				result.push_back((char)(cValue + 44));
			}
			else if (cValue < 40) {  // A - Z
				result.push_back((char)(cValue + 51));
			}
			else {
				return false;
			}
		}
	} while (bits.available() > 0);
	return true;
}

/**
* See ISO 16022:2006, 5.2.8 and Annex C Table C.3
*/
static bool DecodeEdifactSegment(BitSource& bits, std::string& result) {
	do {
		// If there is only two or less bytes left then it will be encoded as ASCII
		if (bits.available() <= 16) {
			return true;
		}

		for (int i = 0; i < 4; i++) {
			int edifactValue = bits.readBits(6);

			// Check for the unlatch character
			if (edifactValue == 0x1F) {  // 011111
										 // Read rest of byte, which should be 0, and stop
				int bitsLeft = 8 - bits.bitOffset();
				if (bitsLeft != 8) {
					bits.readBits(bitsLeft);
				}
				return true;
			}

			if ((edifactValue & 0x20) == 0) {  // no 1 in the leading (6th) bit
				edifactValue |= 0x40;  // Add a leading 01 to the 6 bit binary value
			}
			result.push_back((char)edifactValue);
		}
	} while (bits.available() > 0);
	return true;
}

/**
* See ISO 16022:2006, Annex B, B.2
*/
static int Unrandomize255State(int randomizedBase256Codeword, int base256CodewordPosition)
{
	int pseudoRandomNumber = ((149 * base256CodewordPosition) % 255) + 1;
	int tempVariable = randomizedBase256Codeword - pseudoRandomNumber;
	return tempVariable >= 0 ? tempVariable : tempVariable + 256;
}

/**
* See ISO 16022:2006, 5.2.9 and Annex B, B.2
*/
static bool DecodeBase256Segment(BitSource& bits, std::string& result, std::list<ByteArray>& byteSegments)
{
	// Figure out how long the Base 256 Segment is.
	int codewordPosition = 1 + bits.byteOffset(); // position is 1-indexed
	int d1 = Unrandomize255State(bits.readBits(8), codewordPosition++);
	int count;
	if (d1 == 0) {  // Read the remainder of the symbol
		count = bits.available() / 8;
	}
	else if (d1 < 250) {
		count = d1;
	}
	else {
		count = 250 * (d1 - 249) + Unrandomize255State(bits.readBits(8), codewordPosition++);
	}

	// We're seeing NegativeArraySizeException errors from users.
	if (count < 0) {
		return false;
	}

	ByteArray bytes(count);
	for (int i = 0; i < count; i++) {
		// Have seen this particular error in the wild, such as at
		// http://www.bcgen.com/demo/IDAutomationStreamingDataMatrix.aspx?MODE=3&D=Fred&PFMT=3&PT=F&X=0.3&O=0&LM=0.2
		if (bits.available() < 8) {
			return false;
		}
		bytes[i] = (uint8_t)Unrandomize255State(bits.readBits(8), codewordPosition++);
	}
	byteSegments.push_back(bytes);

	// bytes is in ISO-8859-1
	result.append(bytes.charPtr(), bytes.size());
	return true;
}

ZXING_EXPORT_TEST_ONLY
DecoderResult Decode(ByteArray&& bytes)
{
	BitSource bits(bytes);
	std::string result;
	result.reserve(100);
	std::string resultTrailer;
	std::list<ByteArray> byteSegments;
	Mode mode = Mode::ASCII_ENCODE;
	do {
		if (mode == Mode::ASCII_ENCODE) {
			mode = DecodeAsciiSegment(bits, result, resultTrailer);
		}
		else {
			bool decodeOK;
			switch (mode) {
			case C40_ENCODE:
				decodeOK = DecodeC40Segment(bits, result);
				break;
			case TEXT_ENCODE:
				decodeOK = DecodeTextSegment(bits, result);
				break;
			case ANSIX12_ENCODE:
				decodeOK = DecodeAnsiX12Segment(bits, result);
				break;
			case EDIFACT_ENCODE:
				decodeOK = DecodeEdifactSegment(bits, result);
				break;
			case BASE256_ENCODE:
				decodeOK = DecodeBase256Segment(bits, result, byteSegments);
				break;
			default:
				decodeOK = false;
				break;
			}
			if (!decodeOK) {
				return DecodeStatus::FormatError;
			}
			mode = Mode::ASCII_ENCODE;
		}
	} while (mode != Mode::PAD_ENCODE && bits.available() > 0);

	if (resultTrailer.length() > 0) {
		result.append(resultTrailer);
	}

	return DecoderResult(std::move(bytes), TextDecoder::FromLatin1(result)).setByteSegments(std::move(byteSegments));
}

} // namespace DecodedBitStreamParser

//public DecoderResult decode(boolean[][] image) throws FormatException, ChecksumException{
//	int dimension = image.length;
//BitMatrix bits = new BitMatrix(dimension);
//for (int i = 0; i < dimension; i++) {
//	for (int j = 0; j < dimension; j++) {
//		if (image[i][j]) {
//			bits.set(j, i);
//		}
//	}
//}
//return decode(bits);
//}

/**
* <p>Given data and error-correction codewords received, possibly corrupted by errors, attempts to
* correct the errors in-place using Reed-Solomon error correction.</p>
*
* @param codewordBytes data and error correction codewords
* @param numDataCodewords number of codewords that are data bytes
* @throws ChecksumException if error correction fails
*/
static bool
CorrectErrors(ByteArray& codewordBytes, int numDataCodewords)
{
	// First read into an array of ints
	std::vector<int> codewordsInts(codewordBytes.begin(), codewordBytes.end());
	int numECCodewords = codewordBytes.length() - numDataCodewords;
	if (!ReedSolomonDecoder::Decode(GenericGF::DataMatrixField256(), codewordsInts, numECCodewords))
		return false;

	// Copy back into array of bytes -- only need to worry about the bytes that were data
	// We don't care about errors in the error-correction codewords
	std::copy_n(codewordsInts.begin(), numDataCodewords, codewordBytes.begin());

	return true;
}

DecoderResult Decoder::Decode(const BitMatrix& bits)
{
	// Construct a parser and read version, error-correction level
	const Version* version = BitMatrixParser::ReadVersion(bits);
	if (version == nullptr) {
		return DecodeStatus::FormatError;
	}

	// Read codewords
	ByteArray codewords = BitMatrixParser::ReadCodewords(bits);
	if (codewords.empty())
		return DecodeStatus::FormatError;

	// Separate into data blocks
	std::vector<DataBlock> dataBlocks = DataBlock::GetDataBlocks(codewords, *version);
	if (dataBlocks.empty())
		return DecodeStatus::FormatError;

	// Count total number of data bytes
	int totalBytes = 0;
	for (const DataBlock& db : dataBlocks) {
		totalBytes += db.numDataCodewords();
	}
	ByteArray resultBytes(totalBytes);

	// Error-correct and copy data blocks together into a stream of bytes
	int dataBlocksCount = static_cast<int>(dataBlocks.size());
	for (int j = 0; j < dataBlocksCount; j++) {
		auto& dataBlock = dataBlocks[j];
		ByteArray& codewordBytes = dataBlock.codewords();
		int numDataCodewords = dataBlock.numDataCodewords();
		if (!CorrectErrors(codewordBytes, numDataCodewords))
			return DecodeStatus::ChecksumError;

		for (int i = 0; i < numDataCodewords; i++) {
			// De-interlace data blocks.
			resultBytes[i * dataBlocksCount + j] = codewordBytes[i];
		}
	}

	// Decode the contents of that stream of bytes
	return DecodedBitStreamParser::Decode(std::move(resultBytes));
}

} // DataMatrix
} // ZXing
