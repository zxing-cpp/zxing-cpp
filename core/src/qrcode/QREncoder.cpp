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

#include "qrcode/QREncoder.h"
#include "qrcode/QRMaskUtil.h"
#include "qrcode/QRMatrixUtil.h"
#include "qrcode/QRErrorCorrectionLevel.h"
#include "qrcode/QREncodeResult.h"
#include "GenericGF.h"
#include "ReedSolomonEncoder.h"
#include "BitArray.h"
#include "CharacterSet.h"
#include "CharacterSetECI.h"
#include "TextEncoder.h"
#include "ZXStrConvWorkaround.h"

#include <array>

namespace ZXing {
namespace QRCode {

// The original table is defined in the table 5 of JISX0510:2004 (p.19).
static const std::array<int, 16*6> ALPHANUMERIC_TABLE = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x00-0x0f
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x10-0x1f
	36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  // 0x20-0x2f
	0,   1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1,  // 0x30-0x3f
	-1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // 0x40-0x4f
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1,  // 0x50-0x5f
};

// The mask penalty calculation is complicated.  See Table 21 of JISX0510:2004 (p.45) for details.
// Basically it applies four rules and summate all penalties.
static int CalculateMaskPenalty(const ByteMatrix& matrix)
{
	return MaskUtil::ApplyMaskPenaltyRule1(matrix)
		+ MaskUtil::ApplyMaskPenaltyRule2(matrix)
		+ MaskUtil::ApplyMaskPenaltyRule3(matrix)
		+ MaskUtil::ApplyMaskPenaltyRule4(matrix);
}


static bool IsOnlyDoubleByteKanji(const std::wstring& content)
{
	std::string bytes;
	TextEncoder::GetBytes(content, CharacterSet::Shift_JIS, bytes);
	size_t length = bytes.length();
	if (length % 2 != 0) {
		return false;
	}
	for (size_t i = 0; i < length; i += 2) {
		int byte1 = bytes[i] & 0xFF;
		if ((byte1 < 0x81 || byte1 > 0x9F) && (byte1 < 0xE0 || byte1 > 0xEB)) {
			return false;
		}
	}
	return true;
}

/**
* @return the code point of the table used in alphanumeric mode or
*  -1 if there is no corresponding code in the table.
*/
static int GetAlphanumericCode(int code)
{
	if (code < (int)ALPHANUMERIC_TABLE.size()) {
		return ALPHANUMERIC_TABLE[code];
	}
	return -1;
}

/**
* Choose the best mode by examining the content. Note that 'encoding' is used as a hint;
* if it is Shift_JIS, and the input is only double-byte Kanji, then we return {@link Mode#KANJI}.
*/
static CodecMode::Mode ChooseMode(const std::wstring& content, CharacterSet encoding)
{
	if (encoding == CharacterSet::Shift_JIS && IsOnlyDoubleByteKanji(content)) {
		// Choose Kanji mode if all input are double-byte characters
		return CodecMode::KANJI;
	}
	bool hasNumeric = false;
	bool hasAlphanumeric = false;
	for (wchar_t c : content) {
		if (c >= '0' && c <= '9') {
			hasNumeric = true;
		}
		else if (GetAlphanumericCode(c) != -1) {
			hasAlphanumeric = true;
		}
		else {
			return CodecMode::BYTE;
		}
	}
	if (hasAlphanumeric) {
		return CodecMode::ALPHANUMERIC;
	}
	if (hasNumeric) {
		return CodecMode::NUMERIC;
	}
	return CodecMode::BYTE;
}

static void AppendECI(CharacterSet eci, BitArray& bits)
{
	bits.appendBits(CodecMode::ECI, 4);
	// This is correct for values up to 127, which is all we need now.
	bits.appendBits(CharacterSetECI::ValueForCharset(eci), 8);
}

/**
* Append mode info. On success, store the result in "bits".
*/
static void AppendModeInfo(CodecMode::Mode mode, BitArray& bits)
{
	bits.appendBits(mode, 4);
}


/**
* Append length info. On success, store the result in "bits".
*/
static void AppendLengthInfo(int numLetters, const Version& version, CodecMode::Mode mode, BitArray& bits)
{
	int numBits = CodecMode::CharacterCountBits(mode, version);
	if (numLetters >= (1 << numBits)) {
		return throw std::invalid_argument(std::to_string(numLetters) + " is bigger than " + std::to_string((1 << numBits) - 1));
	}
	bits.appendBits(numLetters, numBits);
}

static void AppendNumericBytes(const std::wstring& content, BitArray& bits)
{
	size_t length = content.length();
	size_t i = 0;
	while (i < length) {
		int num1 = content[i] - '0';
		if (i + 2 < length) {
			// Encode three numeric letters in ten bits.
			int num2 = content[i + 1] - '0';
			int num3 = content[i + 2] - '0';
			bits.appendBits(num1 * 100 + num2 * 10 + num3, 10);
			i += 3;
		}
		else if (i + 1 < length) {
			// Encode two numeric letters in seven bits.
			int num2 = content[i + 1] - '0';
			bits.appendBits(num1 * 10 + num2, 7);
			i += 2;
		}
		else {
			// Encode one numeric letter in four bits.
			bits.appendBits(num1, 4);
			i++;
		}
	}
}

static void AppendAlphanumericBytes(const std::wstring& content, BitArray& bits)
{
	size_t length = content.length();
	size_t i = 0;
	while (i < length) {
		int code1 = GetAlphanumericCode(content[i]);
		if (code1 == -1) {
			throw std::invalid_argument("Unexpected contents");
		}
		if (i + 1 < length) {
			int code2 = GetAlphanumericCode(content[i + 1]);
			if (code2 == -1) {
				throw std::invalid_argument("Unexpected contents");
			}
			// Encode two alphanumeric letters in 11 bits.
			bits.appendBits(code1 * 45 + code2, 11);
			i += 2;
		}
		else {
			// Encode one alphanumeric letter in six bits.
			bits.appendBits(code1, 6);
			i++;
		}
	}
}

static void Append8BitBytes(const std::wstring& content, CharacterSet encoding, BitArray& bits)
{
	std::string bytes;
	TextEncoder::GetBytes(content, encoding, bytes);
	for (char b : bytes) {
		bits.appendBits(b, 8);
	}
}

static void AppendKanjiBytes(const std::wstring& content, BitArray& bits)
{
	std::string bytes;
	TextEncoder::GetBytes(content, CharacterSet::Shift_JIS, bytes);
	size_t length = bytes.size();
	for (size_t i = 0; i < length; i += 2) {
		int byte1 = bytes[i] & 0xFF;
		int byte2 = bytes[i + 1] & 0xFF;
		int code = (byte1 << 8) | byte2;
		int subtracted = -1;
		if (code >= 0x8140 && code <= 0x9ffc) {
			subtracted = code - 0x8140;
		}
		else if (code >= 0xe040 && code <= 0xebbf) {
			subtracted = code - 0xc140;
		}
		if (subtracted == -1) {
			throw std::invalid_argument("Invalid byte sequence");
		}
		int encoded = ((subtracted >> 8) * 0xc0) + (subtracted & 0xff);
		bits.appendBits(encoded, 13);
	}
}
/**
* Append "bytes" in "mode" mode (encoding) into "bits". On success, store the result in "bits".
*/
static void AppendBytes(const std::wstring& content, CodecMode::Mode mode, CharacterSet encoding, BitArray& bits)
{
	switch (mode) {
		case CodecMode::NUMERIC:
			AppendNumericBytes(content, bits);
			break;
		case CodecMode::ALPHANUMERIC:
			AppendAlphanumericBytes(content, bits);
			break;
		case CodecMode::BYTE:
			Append8BitBytes(content, encoding, bits);
			break;
		case CodecMode::KANJI:
			AppendKanjiBytes(content, bits);
			break;
		default:
			throw std::invalid_argument("Invalid mode: " + std::to_string(mode));
	}
}

static const Version& ChooseVersion(int numInputBits, ErrorCorrectionLevel ecLevel)
{
	// In the following comments, we use numbers of Version 7-H.
	for (int versionNum = 1; versionNum <= 40; versionNum++) {
		const Version* version = Version::VersionForNumber(versionNum);
		// numBytes = 196
		int numBytes = version->totalCodewords();
		// getNumECBytes = 130
		auto& ecBlocks = version->ecBlocksForLevel(ecLevel);
		int numEcBytes = ecBlocks.totalCodewords();
		// getNumDataBytes = 196 - 130 = 66
		int numDataBytes = numBytes - numEcBytes;
		int totalInputBytes = (numInputBits + 7) / 8;
		if (numDataBytes >= totalInputBytes) {
			return *version;
		}
	}
	throw std::invalid_argument("Data too big");
}

/**
* Terminate bits as described in 8.4.8 and 8.4.9 of JISX0510:2004 (p.24).
*/
static void TerminateBits(int numDataBytes, BitArray& bits)
{
	int capacity = numDataBytes * 8;
	if (bits.size() > capacity) {
		throw std::invalid_argument("data bits cannot fit in the QR Code" + std::to_string(bits.size()) + " > " + std::to_string(capacity));
	}
	for (int i = 0; i < 4 && bits.size() < capacity; ++i) {
		bits.appendBit(false);
	}
	// Append termination bits. See 8.4.8 of JISX0510:2004 (p.24) for details.
	// If the last byte isn't 8-bit aligned, we'll add padding bits.
	int numBitsInLastByte = bits.size() & 0x07;
	if (numBitsInLastByte > 0) {
		for (int i = numBitsInLastByte; i < 8; i++) {
			bits.appendBit(false);
		}
	}
	// If we have more space, we'll fill the space with padding patterns defined in 8.4.9 (p.24).
	int numPaddingBytes = numDataBytes - bits.sizeInBytes();
	for (int i = 0; i < numPaddingBytes; ++i) {
		bits.appendBits((i & 0x01) == 0 ? 0xEC : 0x11, 8);
	}
	if (bits.size() != capacity) {
		throw std::invalid_argument("Bits size does not equal capacity");
	}
}

struct BlockPair
{
	ByteArray dataBytes;
	ByteArray ecBytes;
};



/**
* Get number of data bytes and number of error correction bytes for block id "blockID". Store
* the result in "numDataBytesInBlock", and "numECBytesInBlock". See table 12 in 8.5.1 of
* JISX0510:2004 (p.30)
*/
static void GetNumDataBytesAndNumECBytesForBlockID(int numTotalBytes, int numDataBytes, int numRSBlocks, int blockID,  int& numDataBytesInBlock, int& numECBytesInBlock)
{
	if (blockID >= numRSBlocks) {
		throw std::invalid_argument("Block ID too large");
	}
	// numRsBlocksInGroup2 = 196 % 5 = 1
	int numRsBlocksInGroup2 = numTotalBytes % numRSBlocks;
	// numRsBlocksInGroup1 = 5 - 1 = 4
	int numRsBlocksInGroup1 = numRSBlocks - numRsBlocksInGroup2;
	// numTotalBytesInGroup1 = 196 / 5 = 39
	int numTotalBytesInGroup1 = numTotalBytes / numRSBlocks;
	// numTotalBytesInGroup2 = 39 + 1 = 40
	int numTotalBytesInGroup2 = numTotalBytesInGroup1 + 1;
	// numDataBytesInGroup1 = 66 / 5 = 13
	int numDataBytesInGroup1 = numDataBytes / numRSBlocks;
	// numDataBytesInGroup2 = 13 + 1 = 14
	int numDataBytesInGroup2 = numDataBytesInGroup1 + 1;
	// numEcBytesInGroup1 = 39 - 13 = 26
	int numEcBytesInGroup1 = numTotalBytesInGroup1 - numDataBytesInGroup1;
	// numEcBytesInGroup2 = 40 - 14 = 26
	int numEcBytesInGroup2 = numTotalBytesInGroup2 - numDataBytesInGroup2;
	// Sanity checks.
	// 26 = 26
	if (numEcBytesInGroup1 != numEcBytesInGroup2) {
		throw std::invalid_argument("EC bytes mismatch");
	}
	// 5 = 4 + 1.
	if (numRSBlocks != numRsBlocksInGroup1 + numRsBlocksInGroup2) {
		throw std::invalid_argument("RS blocks mismatch");
	}
	// 196 = (13 + 26) * 4 + (14 + 26) * 1
	if (numTotalBytes !=
		((numDataBytesInGroup1 + numEcBytesInGroup1) *
			numRsBlocksInGroup1) +
			((numDataBytesInGroup2 + numEcBytesInGroup2) *
				numRsBlocksInGroup2)) {
		throw std::invalid_argument("Total bytes mismatch");
	}

	if (blockID < numRsBlocksInGroup1) {
		numDataBytesInBlock = numDataBytesInGroup1;
		numECBytesInBlock = numEcBytesInGroup1;
	}
	else {
		numDataBytesInBlock = numDataBytesInGroup2;
		numECBytesInBlock = numEcBytesInGroup2;
	}
}

static void GenerateECBytes(const ByteArray& dataBytes, int numEcBytesInBlock, ByteArray& ecBytes)
{
	size_t numDataBytes = dataBytes.size();
	std::vector<int> toEncode(numDataBytes + numEcBytesInBlock, 0);
	for (size_t i = 0; i < numDataBytes; i++) {
		toEncode[i] = dataBytes[i] & 0xFF;
	}
	ReedSolomonEncoder(GenericGF::QRCodeField256()).encode(toEncode, numEcBytesInBlock);

	ecBytes.resize(numEcBytesInBlock);
	for (int i = 0; i < numEcBytesInBlock; i++) {
		ecBytes[i] = static_cast<uint8_t>(toEncode[numDataBytes + i]);
	}
}


/**
* Interleave "bits" with corresponding error correction bytes. On success, store the result in
* "result". The interleave rule is complicated. See 8.6 of JISX0510:2004 (p.37) for details.
*/
static void InterleaveWithECBytes(const BitArray& bits, int numTotalBytes, int numDataBytes, int numRSBlocks, BitArray& output)
{
	// "bits" must have "getNumDataBytes" bytes of data.
	if (bits.sizeInBytes() != numDataBytes) {
		throw std::invalid_argument("Number of bits and data bytes does not match");
	}

	// Step 1.  Divide data bytes into blocks and generate error correction bytes for them. We'll
	// store the divided data bytes blocks and error correction bytes blocks into "blocks".
	int dataBytesOffset = 0;
	int maxNumDataBytes = 0;
	int maxNumEcBytes = 0;

	// Since, we know the number of reedsolmon blocks, we can initialize the vector with the number.
	std::vector<BlockPair> blocks(numRSBlocks);

	for (int i = 0; i < numRSBlocks; ++i) {
		int numDataBytesInBlock = 0;
		int numEcBytesInBlock = 0;
		GetNumDataBytesAndNumECBytesForBlockID(numTotalBytes, numDataBytes, numRSBlocks, i, numDataBytesInBlock, numEcBytesInBlock);

		int size = numDataBytesInBlock;
		blocks[i].dataBytes.resize(size);
		bits.toBytes(8 * dataBytesOffset, blocks[i].dataBytes.data(), size);
		GenerateECBytes(blocks[i].dataBytes, numEcBytesInBlock, blocks[i].ecBytes);

		maxNumDataBytes = std::max(maxNumDataBytes, size);
		maxNumEcBytes = std::max(maxNumEcBytes, (int)blocks[i].ecBytes.size());
		dataBytesOffset += numDataBytesInBlock;
	}
	if (numDataBytes != dataBytesOffset) {
		throw std::invalid_argument("Data bytes does not match offset");
	}

	// First, place data blocks.
	for (int i = 0; i < maxNumDataBytes; ++i) {
		for (auto& block : blocks) {
			if (i < (int)block.dataBytes.size()) {
				output.appendBits(block.dataBytes[i], 8);
			}
		}
	}
	// Then, place error correction blocks.
	for (int i = 0; i < maxNumEcBytes; ++i) {
		for (auto& block : blocks) {
			if (i < (int)block.ecBytes.size()) {
				output.appendBits(block.ecBytes[i], 8);
			}
		}
	}
	if (numTotalBytes != output.sizeInBytes()) {  // Should be same.
		throw std::invalid_argument("Interleaving error: " + std::to_string(numTotalBytes) + " and " + std::to_string(output.sizeInBytes()) + " differ.");
	}
}


static int ChooseMaskPattern(const BitArray& bits, ErrorCorrectionLevel ecLevel, const Version& version, ByteMatrix& matrix)
{
	int minPenalty = std::numeric_limits<int>::max();  // Lower penalty is better.
	int bestMaskPattern = -1;
	// We try all mask patterns to choose the best one.
	for (int maskPattern = 0; maskPattern < MatrixUtil::NUM_MASK_PATTERNS; maskPattern++) {
		MatrixUtil::BuildMatrix(bits, ecLevel, version, maskPattern, matrix);
		int penalty = CalculateMaskPenalty(matrix);
		if (penalty < minPenalty) {
			minPenalty = penalty;
			bestMaskPattern = maskPattern;
		}
	}
	return bestMaskPattern;
}

void
Encoder::Encode(const std::wstring& content, ErrorCorrectionLevel ecLevel, CharacterSet charset, EncodeResult& output)
{
	// Pick an encoding mode appropriate for the content. Note that this will not attempt to use
	// multiple modes / segments even if that were more efficient. Twould be nice.
	CodecMode::Mode mode = ChooseMode(content, charset);

	// This will store the header information, like mode and
	// length, as well as "header" segments like an ECI segment.
	BitArray headerBits;

	// Append ECI segment if applicable
	if (mode == CodecMode::BYTE && charset != DEFAULT_BYTE_MODE_ENCODING) {
		AppendECI(charset, headerBits);
	}

	// (With ECI in place,) Write the mode marker
	AppendModeInfo(mode, headerBits);

	// Collect data within the main segment, separately, to count its size if needed. Don't add it to
	// main payload yet.
	BitArray dataBits;
	AppendBytes(content, mode, charset, dataBits);

	// Hard part: need to know version to know how many bits length takes. But need to know how many
	// bits it takes to know version. First we take a guess at version by assuming version will be
	// the minimum, 1:

	int provisionalBitsNeeded = headerBits.size()
		+ CodecMode::CharacterCountBits(mode, *Version::VersionForNumber(1))
		+ dataBits.size();
	const Version& provisionalVersion = ChooseVersion(provisionalBitsNeeded, ecLevel);

	// Use that guess to calculate the right version. I am still not sure this works in 100% of cases.

	int bitsNeeded = headerBits.size()
		+ CodecMode::CharacterCountBits(mode, provisionalVersion)
		+ dataBits.size();
	const Version& version = ChooseVersion(bitsNeeded, ecLevel);

	BitArray headerAndDataBits;
	headerAndDataBits.appendBitArray(headerBits);
	// Find "length" of main segment and write it
	int numLetters = mode == CodecMode::BYTE ? dataBits.sizeInBytes() : content.length();
	AppendLengthInfo(numLetters, version, mode, headerAndDataBits);
	// Put data together into the overall payload
	headerAndDataBits.appendBitArray(dataBits);

	auto& ecBlocks = version.ecBlocksForLevel(ecLevel);
	int numDataBytes = version.totalCodewords() - ecBlocks.totalCodewords();

	// Terminate the bits properly.
	TerminateBits(numDataBytes, headerAndDataBits);

	// Interleave data bits with error correction code.
	BitArray finalBits;
	InterleaveWithECBytes(headerAndDataBits, version.totalCodewords(), numDataBytes, ecBlocks.numBlocks(), finalBits);

	output.ecLevel = ecLevel;
	output.mode = mode;
	output.version = &version;

	//  Choose the mask pattern and set to "qrCode".
	int dimension = version.dimensionForVersion();
	output.matrix.init(dimension, dimension);
	output.maskPattern = ChooseMaskPattern(finalBits, ecLevel, version, output.matrix);

	// Build the matrix and set it to "qrCode".
	MatrixUtil::BuildMatrix(finalBits, ecLevel, version, output.maskPattern, output.matrix);
}

} // QRCode
} // ZXing
