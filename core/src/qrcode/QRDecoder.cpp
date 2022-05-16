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

#include "QRDecoder.h"

#include "BitMatrix.h"
#include "BitSource.h"
#include "CharacterSet.h"
#include "CharacterSetECI.h"
#include "DecodeStatus.h"
#include "DecoderResult.h"
#include "GenericGF.h"
#include "QRBitMatrixParser.h"
#include "QRCodecMode.h"
#include "QRDataBlock.h"
#include "QRFormatInformation.h"
#include "QRVersion.h"
#include "ReedSolomonDecoder.h"
#include "StructuredAppend.h"
#include "TextDecoder.h"
#include "ZXContainerAlgorithms.h"
#include "ZXTestSupport.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ZXing::QRCode {

/**
* <p>Given data and error-correction codewords received, possibly corrupted by errors, attempts to
* correct the errors in-place using Reed-Solomon error correction.</p>
*
* @param codewordBytes data and error correction codewords
* @param numDataCodewords number of codewords that are data bytes
* @throws ChecksumException if error correction fails
*/
static bool CorrectErrors(ByteArray& codewordBytes, int numDataCodewords)
{
	// First read into an array of ints
	std::vector<int> codewordsInts(codewordBytes.begin(), codewordBytes.end());

	int numECCodewords = Size(codewordBytes) - numDataCodewords;
	if (!ReedSolomonDecode(GenericGF::QRCodeField256(), codewordsInts, numECCodewords))
		return false;

	// Copy back into array of bytes -- only need to worry about the bytes that were data
	// We don't care about errors in the error-correction codewords
	std::copy_n(codewordsInts.begin(), numDataCodewords, codewordBytes.begin());
	return true;
}


/**
* See specification GBT 18284-2000
*/
static DecodeStatus DecodeHanziSegment(BitSource& bits, int count, std::wstring& result)
{
	// Don't crash trying to read more bits than we have available.
	if (count * 13 > bits.available()) {
		return DecodeStatus::FormatError;
	}

	// Each character will require 2 bytes. Read the characters as 2-byte pairs
	// and decode as GB2312 afterwards
	ByteArray buffer;
	buffer.reserve(2 * count);
	while (count > 0) {
		// Each 13 bits encodes a 2-byte character
		int twoBytes = bits.readBits(13);
		int assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
		if (assembledTwoBytes < 0x00A00) {
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

	TextDecoder::Append(result, buffer.data(), Size(buffer), CharacterSet::GB2312);
	return DecodeStatus::NoError;
}

static DecodeStatus DecodeKanjiSegment(BitSource& bits, int count, std::wstring& result)
{
	// Don't crash trying to read more bits than we have available.
	if (count * 13 > bits.available()) {
		return DecodeStatus::FormatError;
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

	TextDecoder::Append(result, buffer.data(), Size(buffer), CharacterSet::Shift_JIS);
	return DecodeStatus::NoError;
}

static DecodeStatus DecodeByteSegment(BitSource& bits, int count, CharacterSet currentCharset, const std::string& hintedCharset,
									  std::wstring& result)
{
	// Don't crash trying to read more bits than we have available.
	if (8 * count > bits.available()) {
		return DecodeStatus::FormatError;
	}

	ByteArray readBytes(count);
	for (int i = 0; i < count; i++) {
		readBytes[i] = static_cast<uint8_t>(bits.readBits(8));
	}
 	if (currentCharset == CharacterSet::Unknown) {
		// The spec isn't clear on this mode; see
		// section 6.4.5: t does not say which encoding to assuming
		// upon decoding. I have seen ISO-8859-1 used as well as
		// Shift_JIS -- without anything like an ECI designator to
		// give a hint.
		if (!hintedCharset.empty())
		{
			currentCharset = CharacterSetECI::CharsetFromName(hintedCharset.c_str());
		}
		if (currentCharset == CharacterSet::Unknown)
		{
			currentCharset = TextDecoder::GuessEncoding(readBytes.data(), Size(readBytes));
		}
	}
	TextDecoder::Append(result, readBytes.data(), Size(readBytes), currentCharset);
	return DecodeStatus::NoError;
}

static char ToAlphaNumericChar(int value)
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

	if (value < 0 || value >= Size(ALPHANUMERIC_CHARS)) {
		throw std::out_of_range("ToAlphaNumericChar: out of range");
	}
	return ALPHANUMERIC_CHARS[value];
}

static DecodeStatus DecodeAlphanumericSegment(BitSource& bits, int count, bool fc1InEffect, std::wstring& result)
{
	// Read two characters at a time
	std::string buffer;
	while (count > 1) {
		if (bits.available() < 11) {
			return DecodeStatus::FormatError;
		}
		int nextTwoCharsBits = bits.readBits(11);
		buffer += ToAlphaNumericChar(nextTwoCharsBits / 45);
		buffer += ToAlphaNumericChar(nextTwoCharsBits % 45);
		count -= 2;
	}
	if (count == 1) {
		// special case: one character left
		if (bits.available() < 6) {
			return DecodeStatus::FormatError;
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
	TextDecoder::AppendLatin1(result, buffer);
	return DecodeStatus::NoError;
}

static DecodeStatus DecodeNumericSegment(BitSource& bits, int count, std::wstring& result)
{
	// Read three digits at a time
	std::string buffer;
	while (count >= 3) {
		// Each 10 bits encodes three digits
		if (bits.available() < 10) {
			return DecodeStatus::FormatError;
		}
		int threeDigitsBits = bits.readBits(10);
		if (threeDigitsBits >= 1000) {
			return DecodeStatus::FormatError;
		}
		buffer += ToAlphaNumericChar(threeDigitsBits / 100);
		buffer += ToAlphaNumericChar((threeDigitsBits / 10) % 10);
		buffer += ToAlphaNumericChar(threeDigitsBits % 10);
		count -= 3;
	}
	if (count == 2) {
		// Two digits left over to read, encoded in 7 bits
		if (bits.available() < 7) {
			return DecodeStatus::FormatError;
		}
		int twoDigitsBits = bits.readBits(7);
		if (twoDigitsBits >= 100) {
			return DecodeStatus::FormatError;
		}
		buffer += ToAlphaNumericChar(twoDigitsBits / 10);
		buffer += ToAlphaNumericChar(twoDigitsBits % 10);
	}
	else if (count == 1) {
		// One digit left over to read
		if (bits.available() < 4) {
			return DecodeStatus::FormatError;
		}
		int digitBits = bits.readBits(4);
		if (digitBits >= 10) {
			return DecodeStatus::FormatError;
		}
		buffer += ToAlphaNumericChar(digitBits);
	}

	TextDecoder::AppendLatin1(result, buffer);
	return DecodeStatus::NoError;
}

static DecodeStatus ParseECIValue(BitSource& bits, int& outValue)
{
	int firstByte = bits.readBits(8);
	if ((firstByte & 0x80) == 0) {
		// just one byte
		outValue = firstByte & 0x7F;
		return DecodeStatus::NoError;
	}
	if ((firstByte & 0xC0) == 0x80) {
		// two bytes
		int secondByte = bits.readBits(8);
		outValue = ((firstByte & 0x3F) << 8) | secondByte;
		return DecodeStatus::NoError;
	}
	if ((firstByte & 0xE0) == 0xC0) {
		// three bytes
		int secondThirdBytes = bits.readBits(16);
		outValue = ((firstByte & 0x1F) << 16) | secondThirdBytes;
		return DecodeStatus::NoError;
	}
	return DecodeStatus::FormatError;
}

/**
 * QR codes encode mode indicators and terminator codes into a constant bit length of 4.
 * Micro QR codes have terminator codes that vary in bit length but are always longer than
 * the mode indicators.
 * M1 - 0 length mode code, 3 bits terminator code
 * M2 - 1 bit mode code, 5 bits terminator code
 * M3 - 2 bit mode code, 7 bits terminator code
 * M4 - 3 bit mode code, 9 bits terminator code
 * IsTerminator peaks into the bit stream to see if the current position is at the start of
 * a terminator code.  If true, then the decoding can finish. If false, then the decoding
 * can read off the next mode code.
 *
 * See ISO 18004:2006, 6.4.1 Table 2
 *
 * @param bits the stream of bits that might have a terminator code
 * @param version the QR or micro QR code version
 */
bool IsTerminator(const BitSource& bits, const Version& version)
{
	const int bitsRequired = TerminatorBitsLength(version);
	const int bitsAvailable = std::min(bits.available(), bitsRequired);
	return bits.peakBits(bitsAvailable) == 0;
}

/**
* <p>QR Codes can encode text as bits in one of several modes, and can use multiple modes
* in one QR Code. This method decodes the bits back into text.</p>
*
* <p>See ISO 18004:2006, 6.4.3 - 6.4.7</p>
*/
ZXING_EXPORT_TEST_ONLY
DecoderResult DecodeBitStream(ByteArray&& bytes, const Version& version, ErrorCorrectionLevel ecLevel,
							  const std::string& hintedCharset)
{
	BitSource bits(bytes);
	std::wstring result;
	int symbologyIdModifier = 1; // ISO/IEC 18004:2015 Annex F Table F.1
	int appIndValue = -1; // ISO/IEC 18004:2015 7.4.8.3 AIM Application Indicator (FNC1 in second position)
	StructuredAppendInfo structuredAppend;
	static const int GB2312_SUBSET = 1;
	const int modeBitLength = CodecModeBitsLength(version);
	const int minimumBitsRequired = modeBitLength + CharacterCountBits(CodecMode::NUMERIC, version);

	try
	{
		CharacterSet currentCharset = CharacterSet::Unknown;
		bool fc1InEffect = false;
		CodecMode mode;
		do {
			// While still another segment to read...
			if (bits.available() < minimumBitsRequired || IsTerminator(bits, version)) {
				// OK, assume we're done. Really, a TERMINATOR mode should have been recorded here
				mode = CodecMode::TERMINATOR;
			} else if (modeBitLength == 0) {
				// MicroQRCode version 1 is always NUMERIC and modeBitLength is 0
				mode = CodecMode::NUMERIC;
			} else {
				mode = CodecModeForBits(bits.readBits(modeBitLength), version.isMicroQRCode());
			}
			switch (mode) {
			case CodecMode::TERMINATOR:
				break;
			case CodecMode::FNC1_FIRST_POSITION:
				fc1InEffect = true; // In Alphanumeric mode undouble doubled percents and treat single percent as <GS>
				// As converting character set ECIs ourselves and ignoring/skipping non-character ECIs, not using
				// modifiers that indicate ECI protocol (ISO/IEC 18004:2015 Annex F Table F.1)
				symbologyIdModifier = 3;
				break;
			case CodecMode::FNC1_SECOND_POSITION:
				fc1InEffect = true; // As above
				symbologyIdModifier = 5; // As above
				// ISO/IEC 18004:2015 7.4.8.3 AIM Application Indicator "00-99" or "A-Za-z"
				appIndValue = bits.readBits(8); // Number 00-99 or ASCII value + 100; prefixed to data below
				break;
			case CodecMode::STRUCTURED_APPEND:
				if (bits.available() < 16) {
					return DecodeStatus::FormatError;
				}
				// sequence number and parity is added later to the result metadata
				// Read next 4 bits of index, 4 bits of symbol count, and 8 bits of parity data, then continue
				structuredAppend.index = bits.readBits(4);
				structuredAppend.count = bits.readBits(4) + 1;
				structuredAppend.id    = std::to_string(bits.readBits(8));
				break;
			case CodecMode::ECI: {
				// Count doesn't apply to ECI
				int value;
				auto status = ParseECIValue(bits, value);
				if (StatusIsError(status)) {
					return status;
				}
				currentCharset = CharacterSetECI::CharsetFromValue(value);
				if (currentCharset == CharacterSet::Unknown) {
					return DecodeStatus::FormatError;
				}
				break;
			}
			case CodecMode::HANZI: {
				// First handle Hanzi mode which does not start with character count
				// chinese mode contains a sub set indicator right after mode indicator
				int subset = bits.readBits(4);
				int countHanzi = bits.readBits(CharacterCountBits(mode, version));
				if (subset == GB2312_SUBSET) {
					auto status = DecodeHanziSegment(bits, countHanzi, result);
					if (StatusIsError(status)) {
						return status;
					}
				}
				break;
			}
			default: {
				// "Normal" QR code modes:
				// How many characters will follow, encoded in this mode?
				int count = bits.readBits(CharacterCountBits(mode, version));
				DecodeStatus status;
				switch (mode) {
				case CodecMode::NUMERIC:      status = DecodeNumericSegment(bits, count, result); break;
				case CodecMode::ALPHANUMERIC: status = DecodeAlphanumericSegment(bits, count, fc1InEffect, result); break;
				case CodecMode::BYTE:         status = DecodeByteSegment(bits, count, currentCharset, hintedCharset, result); break;
				case CodecMode::KANJI:        status = DecodeKanjiSegment(bits, count, result); break;
				default:                      status = DecodeStatus::FormatError;
				}
				if (StatusIsError(status)) {
					return status;
				}
				break;
			}
			}
		} while (mode != CodecMode::TERMINATOR);
	}
	catch (const std::exception &)
	{
		// from readBits() calls
		return DecodeStatus::FormatError;
	}

	if (appIndValue >= 0) {
		if (appIndValue < 10) { // "00-09"
			result.insert(0, L'0' + std::to_wstring(appIndValue));
		}
		else if (appIndValue < 100) { // "10-99"
			result.insert(0, std::to_wstring(appIndValue));
		}
		else if ((appIndValue >= 165 && appIndValue <= 190) || (appIndValue >= 197 && appIndValue <= 222)) { // "A-Za-z"
			result.insert(0, 1, static_cast<wchar_t>(appIndValue - 100));
		}
		else {
			return DecodeStatus::FormatError;
		}
	}

	std::string symbologyIdentifier("]Q" + std::to_string(symbologyIdModifier));

	return DecoderResult(std::move(bytes), std::move(result))
		.setEcLevel(ToString(ecLevel))
		.setSymbologyIdentifier(std::move(symbologyIdentifier))
		.setStructuredAppend(structuredAppend);
}

static DecoderResult DoDecode(const BitMatrix& bits, const Version& version, const std::string& hintedCharset, bool mirrored)
{
	auto formatInfo = ReadFormatInformation(bits, mirrored, version.isMicroQRCode());
	if (!formatInfo.isValid())
		return DecodeStatus::FormatError;

	// Read codewords
	ByteArray codewords = ReadCodewords(bits, version, formatInfo, mirrored);
	if (codewords.empty())
		return DecodeStatus::FormatError;

	// Separate into data blocks
	std::vector<DataBlock> dataBlocks = DataBlock::GetDataBlocks(codewords, version, formatInfo.errorCorrectionLevel());
	if (dataBlocks.empty())
		return DecodeStatus::FormatError;

	// Count total number of data bytes
	const auto op = [](auto totalBytes, const auto& dataBlock){ return totalBytes + dataBlock.numDataCodewords();};
	const auto totalBytes = std::accumulate(std::begin(dataBlocks), std::end(dataBlocks), int{}, op);
	ByteArray resultBytes(totalBytes);
	auto resultIterator = resultBytes.begin();

	// Error-correct and copy data blocks together into a stream of bytes
	for (auto& dataBlock : dataBlocks)
	{
		ByteArray& codewordBytes = dataBlock.codewords();
		int numDataCodewords = dataBlock.numDataCodewords();

		if (!CorrectErrors(codewordBytes, numDataCodewords))
			return DecodeStatus::ChecksumError;

		resultIterator = std::copy_n(codewordBytes.begin(), numDataCodewords, resultIterator);
	}

	// Decode the contents of that stream of bytes
	return DecodeBitStream(std::move(resultBytes), version, formatInfo.errorCorrectionLevel(), hintedCharset);
}

DecoderResult Decode(const BitMatrix& bits, const std::string& hintedCharset, const bool isMicroQRCode)
{
	const Version* version = ReadVersion(bits, isMicroQRCode);
	if (!version)
		return DecodeStatus::FormatError;

	auto res = DoDecode(bits, *version, hintedCharset, false);
	if (res.isValid())
		return res;

	if (auto resMirrored = DoDecode(bits, *version, hintedCharset, true); resMirrored.isValid()) {
		resMirrored.setIsMirrored(true);
		return resMirrored;
	}

	return res;
}

} // namespace ZXing::QRCode
