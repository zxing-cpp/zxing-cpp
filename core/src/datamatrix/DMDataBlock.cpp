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

#include "datamatrix/DMDataBlock.h"
#include "datamatrix/DMVersion.h"
#include "DecodeStatus.h"

namespace ZXing {
namespace DataMatrix {

std::vector<DataBlock> DataBlock::GetDataBlocks(const ByteArray& rawCodewords, const Version& version)
{
	// First count the total number of data blocks
	// Now establish DataBlocks of the appropriate size and number of data codewords
	auto& ecBlocks = version.ecBlocks();
	std::vector<DataBlock> result(ecBlocks.numBlocks());
	int numResultBlocks = 0;
	for (auto& ecBlock : ecBlocks.blockArray()) {
		for (int i = 0; i < ecBlock.count; i++) {
			auto& item = result[numResultBlocks++];
			item._numDataCodewords = ecBlock.dataCodewords;
			item._codewords.resize(ecBlocks.codewordsPerBlock + ecBlock.dataCodewords);
		}
	}

	// All blocks have the same amount of data, except that the last n
	// (where n may be 0) have 1 less byte. Figure out where these start.
	// TODO(bbrown): There is only one case where there is a difference for Data Matrix for size 144
	int longerBlocksTotalCodewords = result[0]._codewords.length();
	//int shorterBlocksTotalCodewords = longerBlocksTotalCodewords - 1;

	int longerBlocksNumDataCodewords = longerBlocksTotalCodewords - ecBlocks.codewordsPerBlock;
	int shorterBlocksNumDataCodewords = longerBlocksNumDataCodewords - 1;
	// The last elements of result may be 1 element shorter for 144 matrix
	// first fill out as many elements as all of them have minus 1
	int rawCodewordsOffset = 0;
	for (int i = 0; i < shorterBlocksNumDataCodewords; i++) {
		for (int j = 0; j < numResultBlocks; j++) {
			result[j]._codewords[i] = rawCodewords[rawCodewordsOffset++];
		}
	}

	// Fill out the last data block in the longer ones
	bool specialVersion = version.versionNumber() == 24;
	int numLongerBlocks = specialVersion ? 8 : numResultBlocks;
	for (int j = 0; j < numLongerBlocks; j++) {
		result[j]._codewords[longerBlocksNumDataCodewords - 1] = rawCodewords[rawCodewordsOffset++];
	}

	// Now add in error correction blocks
	int max = result[0]._codewords.length();
	for (int i = longerBlocksNumDataCodewords; i < max; i++) {
		for (int j = 0; j < numResultBlocks; j++) {
			int jOffset = specialVersion ? (j + 8) % numResultBlocks : j;
			int iOffset = specialVersion && jOffset > 7 ? i - 1 : i;
			result[jOffset]._codewords[iOffset] = rawCodewords[rawCodewordsOffset++];
		}
	}

	if (rawCodewordsOffset != rawCodewords.length())
		return {};

	return result;
}

} // DataMatrix
} // ZXing
