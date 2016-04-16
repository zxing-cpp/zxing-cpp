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

#include "qrcode/QRDecoder.h"
#include "qrcode/QRBitMatrixParser.h"
#include "qrcode/QRVersion.h"
#include "qrcode/QRFormatInformation.h"
#include "qrcode/QRDecoderMetadata.h"
#include "qrcode/QRDataMask.h"
#include "qrcode/QRDataBlock.h"
#include "qrcode/QRDecodeMode.h"
#include "DecoderResult.h"
#include "BitMatrix.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"
#include "ZXString.h"
#include "BitSource.h"
#include "StringCodecs.h"
#include "CharacterSetECI.h"
#include "DecodeHints.h"

#include <list>

namespace ZXing {
namespace QRCode {

//public Decoder() {
//	rsDecoder = new ReedSolomonDecoder(GenericGF.QR_CODE_FIELD_256);
//}

//public DecoderResult decode(boolean[][] image) throws ChecksumException, FormatException{
//	return decode(image, null);
//}
//
//public DecoderResult decode(boolean[][] image, Map<DecodeHintType, ? > hints)
//throws ChecksumException, FormatException{
//	int dimension = image.length;
//BitMatrix bits = new BitMatrix(dimension);
//for (int i = 0; i < dimension; i++) {
//	for (int j = 0; j < dimension; j++) {
//		if (image[i][j]) {
//			bits.set(j, i);
//		}
//	}
//}
//return decode(bits, hints);
//}
//
//public DecoderResult decode(BitMatrix bits) throws ChecksumException, FormatException{
//	return decode(bits, null);
//}


namespace {

/**
* <p>Given data and error-correction codewords received, possibly corrupted by errors, attempts to
* correct the errors in-place using Reed-Solomon error correction.</p>
*
* @param codewordBytes data and error correction codewords
* @param numDataCodewords number of codewords that are data bytes
* @throws ChecksumException if error correction fails
*/
bool CorrectErrors(ByteArray& codewordBytes, int numDataCodewords)
{
	int numCodewords = codewordBytes.length();
	// First read into an array of ints
	std::vector<int> codewordsInts(numCodewords);
	for (int i = 0; i < numCodewords; i++) {
		codewordsInts[i] = codewordBytes[i] & 0xFF;
	}
	int numECCodewords = codewordBytes.length() - numDataCodewords;
	if (ReedSolomonDecoder(GenericGF::QRCodeField256()).decode(codewordsInts, numECCodewords))
	{
		// Copy back into array of bytes -- only need to worry about the bytes that were data
		// We don't care about errors in the error-correction codewords
		for (int i = 0; i < numDataCodewords; ++i) {
			codewordBytes[i] = static_cast<uint8_t>(codewordsInts[i]);
		}
		return true;
	}
	return false;
}

#pragma region DecodedBitStreamParser

/**
* See specification GBT 18284-2000
*/
bool DecodeHanziSegment(BitSource& bits, int count, String& result)
{
	// Don't crash trying to read more bits than we have available.
	if (count * 13 > bits.available()) {
		return false;
	}

	// Each character will require 2 bytes. Read the characters as 2-byte pairs
	// and decode as GB2312 afterwards
	ByteArray buffer;
	buffer.reserve(2 * count);
	while (count > 0) {
		// Each 13 bits encodes a 2-byte character
		int twoBytes = bits.readBits(13);
		int assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
		if (assembledTwoBytes < 0x003BF) {
			// In the 0xA1A1 to 0xAAFE range
			assembledTwoBytes += 0x0A1A1;
		}
		else {
			// In the 0xB0A1 to 0xFAFE range
			assembledTwoBytes += 0x0A6A1;
		}
		buffer.push_back(static_cast<uint8_t>((assembledTwoBytes >> 8) & 0xFF));
		buffer.push_back(static_cast<uint8_t>(assembledTwoBytes & 0xFF));
		count--;
	}

	result += StringCodecs::Instance()->toUnicode(buffer.charPtr(), buffer.length(), CharacterSet::GB2312);
	return true;
}

bool DecodeKanjiSegment(BitSource& bits, int count, String& result)
{
	// Don't crash trying to read more bits than we have available.
	if (count * 13 > bits.available()) {
		return false;
	}

	// Each character will require 2 bytes. Read the characters as 2-byte pairs
	// and decode as Shift_JIS afterwards
	ByteArray buffer;
	buffer.reserve(2 * count);
	while (count > 0) {
		// Each 13 bits encodes a 2-byte character
		int twoBytes = bits.readBits(13);
		int assembledTwoBytes = ((twoBytes / 0x0C0) << 8) | (twoBytes % 0x0C0);
		if (assembledTwoBytes < 0x01F00) {
			// In the 0x8140 to 0x9FFC range
			assembledTwoBytes += 0x08140;
		}
		else {
			// In the 0xE040 to 0xEBBF range
			assembledTwoBytes += 0x0C140;
		}
		buffer.push_back(static_cast<uint8_t>(assembledTwoBytes >> 8));
		buffer.push_back(static_cast<uint8_t>(assembledTwoBytes));
		count--;
	}

	result += StringCodecs::Instance()->toUnicode(buffer.charPtr(), buffer.length(), CharacterSet::Shift_JIS);
	return true;
}

bool DecodeByteSegment(BitSource& bits, int count, CharacterSet charset, const DecodeHints* hints, String& result, std::list<ByteArray>& byteSegments)
{
	// Don't crash trying to read more bits than we have available.
	if (8 * count > bits.available()) {
		return false;
	}

	ByteArray readBytes(count);
	for (int i = 0; i < count; i++) {
		readBytes[i] = static_cast<uint8_t>(bits.readBits(8));
	}
	const char* encoding = nullptr;
	if (charset == CharacterSet::Unknown) {
		// The spec isn't clear on this mode; see
		// section 6.4.5: t does not say which encoding to assuming
		// upon decoding. I have seen ISO-8859-1 used as well as
		// Shift_JIS -- without anything like an ECI designator to
		// give a hint.
		if (hints != nullptr) {
			auto encoding = hints->getString(DecodeHint::CHARACTER_SET);
			if (!encoding.empty())
			{
				charset = CharacterSetECI::CharsetFromName(encoding.utf8());
			}
		}
		if (charset == CharacterSet::Unknown)
		{
			charset = StringCodecs::GuessEncoding(readBytes.charPtr(), readBytes.length());
		}
	}
	result += StringCodecs::Instance()->toUnicode(readBytes.charPtr(), readBytes.length(), charset);
	byteSegments.push_back(readBytes);
	return true;
}

char ToAlphaNumericChar(int value)
{
	/**
	* See ISO 18004:2006, 6.4.4 Table 5
	*/
	static const char ALPHANUMERIC_CHARS[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
		'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
		'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		' ', '$', '%', '*', '+', '-', '.', '/', ':'
	};

	if (value >= sizeof(ALPHANUMERIC_CHARS)) {
		throw std::out_of_range("ToAlphaNumericChar: out of range");
	}
	return ALPHANUMERIC_CHARS[value];
}

bool DecodeAlphanumericSegment(BitSource& bits, int count, bool fc1InEffect, String& result)
{
	// Read two characters at a time
	std::string buffer;
	while (count > 1) {
		if (bits.available() < 11) {
			return false;
		}
		int nextTwoCharsBits = bits.readBits(11);
		buffer += ToAlphaNumericChar(nextTwoCharsBits / 45);
		buffer += ToAlphaNumericChar(nextTwoCharsBits % 45);
		count -= 2;
	}
	if (count == 1) {
		// special case: one character left
		if (bits.available() < 6) {
			return false;
		}
		buffer += ToAlphaNumericChar(bits.readBits(6));
	}
	// See section 6.4.8.1, 6.4.8.2
	if (fc1InEffect) {
		// We need to massage the result a bit if in an FNC1 mode:
		for (size_t i = 0; i < buffer.length(); i++) {
			if (buffer[i] == '%') {
				if (i < buffer.length() - 1 && buffer[i + 1] == '%') {
					// %% is rendered as %
					buffer.erase(i + 1);
				}
				else {
					// In alpha mode, % should be converted to FNC1 separator 0x1D
					buffer[i] = static_cast<char>(0x1D);
				}
			}
		}
	}
	result.appendUtf8(buffer.c_str());
	return true;
}

bool DecodeNumericSegment(BitSource& bits, int count, String& result)
{
	// Read three digits at a time
	std::string buffer;
	while (count >= 3) {
		// Each 10 bits encodes three digits
		if (bits.available() < 10) {
			return false;
		}
		int threeDigitsBits = bits.readBits(10);
		if (threeDigitsBits >= 1000) {
			return false;
		}
		buffer += ToAlphaNumericChar(threeDigitsBits / 100);
		buffer += ToAlphaNumericChar((threeDigitsBits / 10) % 10);
		buffer += ToAlphaNumericChar(threeDigitsBits % 10);
		count -= 3;
	}
	if (count == 2) {
		// Two digits left over to read, encoded in 7 bits
		if (bits.available() < 7) {
			return false;
		}
		int twoDigitsBits = bits.readBits(7);
		if (twoDigitsBits >= 100) {
			return false;
		}
		buffer += ToAlphaNumericChar(twoDigitsBits / 10);
		buffer += ToAlphaNumericChar(twoDigitsBits % 10);
	}
	else if (count == 1) {
		// One digit left over to read
		if (bits.available() < 4) {
			return false;
		}
		int digitBits = bits.readBits(4);
		if (digitBits >= 10) {
			return false;
		}
		buffer += ToAlphaNumericChar(digitBits);
	}

	result.appendUtf8(buffer.c_str());
	return true;
}

bool ParseECIValue(BitSource& bits, int &outValue)
{
	int firstByte = bits.readBits(8);
	if ((firstByte & 0x80) == 0) {
		// just one byte
		outValue = firstByte & 0x7F;
		return true;
	}
	if ((firstByte & 0xC0) == 0x80) {
		// two bytes
		int secondByte = bits.readBits(8);
		outValue = ((firstByte & 0x3F) << 8) | secondByte;
		return true;
	}
	if ((firstByte & 0xE0) == 0xC0) {
		// three bytes
		int secondThirdBytes = bits.readBits(16);
		outValue = ((firstByte & 0x1F) << 16) | secondThirdBytes;
		return true;
	}
	return false;
}

/**
* <p>QR Codes can encode text as bits in one of several modes, and can use multiple modes
* in one QR Code. This method decodes the bits back into text.</p>
*
* <p>See ISO 18004:2006, 6.4.3 - 6.4.7</p>
*/
static DecoderResult DecodeBitStream(const ByteArray& bytes, const Version& version, ErrorCorrectionLevel ecLevel, const DecodeHints* hints)
{
	BitSource bits(bytes);
	String result;
	std::list<ByteArray> byteSegments;
	int symbolSequence = -1;
	int parityData = -1;
	static const int GB2312_SUBSET = 1;

	CharacterSet currentCharset = CharacterSet::Unknown;
	bool fc1InEffect = false;
	DecodeMode::Mode mode;
	do {
		// While still another segment to read...
		if (bits.available() < 4) {
			// OK, assume we're done. Really, a TERMINATOR mode should have been recorded here
			mode = DecodeMode::TERMINATOR;
		}
		else {
			mode = DecodeMode::ModeForBits(bits.readBits(4)); // mode is encoded by 4 bits
		}
		if (mode != DecodeMode::TERMINATOR) {
			if (mode == DecodeMode::FNC1_FIRST_POSITION || mode == DecodeMode::FNC1_SECOND_POSITION) {
				// We do little with FNC1 except alter the parsed result a bit according to the spec
				fc1InEffect = true;
			}
			else if (mode == DecodeMode::STRUCTURED_APPEND) {
				if (bits.available() < 16) {
					return DecoderResult();
				}
				// sequence number and parity is added later to the result metadata
				// Read next 8 bits (symbol sequence #) and 8 bits (parity data), then continue
				symbolSequence = bits.readBits(8);
				parityData = bits.readBits(8);
			}
			else if (mode == DecodeMode::ECI) {
				// Count doesn't apply to ECI
				int value;
				if (!ParseECIValue(bits, value))
					return DecoderResult();

				currentCharset = CharacterSetECI::CharsetFromValue(value);
				if (currentCharset == CharacterSet::Unknown) {
					return DecoderResult();
				}
			}
			else {
				// First handle Hanzi mode which does not start with character count
				if (mode == DecodeMode::HANZI) {
					//chinese mode contains a sub set indicator right after mode indicator
					int subset = bits.readBits(4);
					int countHanzi = bits.readBits(DecodeMode::CharacterCountBits(mode, version));
					if (subset == GB2312_SUBSET) {
						if (!DecodeHanziSegment(bits, countHanzi, result))
							return DecoderResult();
					}
				}
				else {
					// "Normal" QR code modes:
					// How many characters will follow, encoded in this mode?
					int count = bits.readBits(DecodeMode::CharacterCountBits(mode, version));
					if (mode == DecodeMode::NUMERIC) {
						if (!DecodeNumericSegment(bits, count, result))
							return DecoderResult();
					}
					else if (mode == DecodeMode::ALPHANUMERIC) {
						if (!DecodeAlphanumericSegment(bits, count, fc1InEffect, result)) {
							return DecoderResult();
						}
					}
					else if (mode == DecodeMode::BYTE) {
						if (!DecodeByteSegment(bits, count, currentCharset, hints, result, byteSegments))
							return DecoderResult();
					}
					else if (mode == DecodeMode::KANJI) {
						if (!DecodeKanjiSegment(bits, count, result))
							return DecoderResult();
					}
					else {
						return DecoderResult();
					}
				}
			}
		}
	} while (mode != DecodeMode::TERMINATOR);

	return DecoderResult(bytes, result, byteSegments, ToString(ecLevel), symbolSequence, parityData);
}

#pragma endregion



DecoderResult DoDecode(const BitMatrix& bits, const Version& version, const FormatInformation& formatInfo, const DecodeHints* hints)
{
	auto ecLevel = formatInfo.errorCorrectionLevel();

	// Read codewords
	auto codewords = BitMatrixParser::ReadCodewords(bits, version);
	// Separate into data blocks
	std::vector<DataBlock> dataBlocks;
	if (!DataBlock::GetDataBlocks(codewords, version, ecLevel, dataBlocks))
		return DecoderResult();

	// Count total number of data bytes
	int totalBytes = 0;
	for (DataBlock dataBlock : dataBlocks) {
		totalBytes += dataBlock.numDataCodewords();
	}
	ByteArray resultBytes(totalBytes);
	int resultOffset = 0;

	// Error-correct and copy data blocks together into a stream of bytes
	for (const DataBlock& dataBlock : dataBlocks)
	{
		ByteArray codewordBytes = dataBlock.codewords();
		int numDataCodewords = dataBlock.numDataCodewords();
		
		if (!CorrectErrors(codewordBytes, numDataCodewords))
			return DecoderResult();

		for (int i = 0; i < numDataCodewords; i++) {
			resultBytes[resultOffset++] = codewordBytes[i];
		}
	}

	// Decode the contents of that stream of bytes
	return DecodeBitStream(resultBytes, version, ecLevel, hints);
}

void ReMask(BitMatrix& bitMatrix, const FormatInformation& formatInfo)
{
	int dimension = bitMatrix.height();
	DataMask(formatInfo.dataMask()).unmaskBitMatrix(bitMatrix, dimension);
}

} // anonymous


DecoderResult
Decoder::Decode(const BitMatrix& bits_, const DecodeHints* hints)
{
	BitMatrix bits = bits_;
	// Construct a parser and read version, error-correction level
	const Version* version;
	FormatInformation formatInfo;
	if (BitMatrixParser::ParseVersionInfo(bits, false, version, formatInfo))
	{
		ReMask(bits, formatInfo);
		auto result = DoDecode(bits, *version, formatInfo, hints);
		if (result.isValid())
		{
			return result;
		}
	}

	if (version != nullptr)
	{
		// Revert the bit matrix
		ReMask(bits, formatInfo);
	}

	if (BitMatrixParser::ParseVersionInfo(bits, true, version, formatInfo))
	{
		/*
		* Since we're here, this means we have successfully detected some kind
		* of version and format information when mirrored. This is a good sign,
		* that the QR code may be mirrored, and we should try once more with a
		* mirrored content.
		*/
		// Prepare for a mirrored reading.
		bits.mirror();

		ReMask(bits, formatInfo);
		auto result = DoDecode(bits, *version, formatInfo, hints);
		if (result.isValid())
		{
			result.setExtra(std::make_shared<DecoderMetadata>(true));
			return result;
		}
	}
	return DecoderResult();
}

} // QRCode
} // ZXing
