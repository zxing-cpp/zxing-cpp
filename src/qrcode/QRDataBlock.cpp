#include "qrcode/QRDataBlock.h"
#include "qrcode/QRVersion.h"
#include "qrcode/ErrorCorrectionLevel.h"

#include <utility>

namespace ZXing {
namespace QRCode {

DataBlock::DataBlock() :
	_numDataCodewords(0)
{
}

bool
DataBlock::GetDataBlocks(const ByteArray& rawCodewords, const Version& version, ErrorCorrectionLevel ecLevel, std::vector<DataBlock>& result)
{
	if (rawCodewords.length() != version.totalCodewords()) {
		return false;
	}

	// Figure out the number and size of data blocks used by this version and
	// error correction level
	auto& ecBlocks = version.ecBlocksForLevel(ecLevel);

	// First count the total number of data blocks
	int totalBlocks = ecBlocks.numBlocks();

	result.resize(totalBlocks);;
	// Now establish DataBlocks of the appropriate size and number of data codewords
	int numResultBlocks = 0;
	for (auto& ecBlock : ecBlocks.blockArray()) {
		for (int i = 0; i < ecBlock.count; i++) {
			auto& item = result[numResultBlocks++];
			item._numDataCodewords = ecBlock.dataCodewords;
			item._codewords.resize(ecBlocks.codewordsPerBlock + ecBlock.dataCodewords);
		}
	}

	// All blocks have the same amount of data, except that the last n
	// (where n may be 0) have 1 more byte. Figure out where these start.
	int shorterBlocksTotalCodewords = result[0]._codewords.length();
	int longerBlocksStartAt = static_cast<int>(result.size()) - 1;
	while (longerBlocksStartAt >= 0) {
		int numCodewords = result[longerBlocksStartAt]._codewords.length();
		if (numCodewords == shorterBlocksTotalCodewords) {
			break;
		}
		longerBlocksStartAt--;
	}
	longerBlocksStartAt++;

	int shorterBlocksNumDataCodewords = shorterBlocksTotalCodewords - ecBlocks.codewordsPerBlock;
	// The last elements of result may be 1 element longer;
	// first fill out as many elements as all of them have
	int rawCodewordsOffset = 0;
	for (int i = 0; i < shorterBlocksNumDataCodewords; i++) {
		for (int j = 0; j < numResultBlocks; j++) {
			result[j]._codewords[i] = rawCodewords[rawCodewordsOffset++];
		}
	}
	// Fill out the last data block in the longer ones
	for (int j = longerBlocksStartAt; j < numResultBlocks; j++) {
		result[j]._codewords[shorterBlocksNumDataCodewords] = rawCodewords[rawCodewordsOffset++];
	}
	// Now add in error correction blocks
	int max = result[0]._codewords.length();
	for (int i = shorterBlocksNumDataCodewords; i < max; i++) {
		for (int j = 0; j < numResultBlocks; j++) {
			int iOffset = j < longerBlocksStartAt ? i : i + 1;
			result[j]._codewords[iOffset] = rawCodewords[rawCodewordsOffset++];
		}
	}
	return true;
}

} // QRCode
} // ZXing
