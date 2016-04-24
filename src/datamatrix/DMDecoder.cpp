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

#include "datamatrix/DMDecoder.h"
#include "datamatrix/DMBitMatrixParser.h"
#include "datamatrix/DMDataBlock.h"
#include "DecoderResult.h"
#include "BitMatrix.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"

namespace ZXing {
namespace DataMatrix {

//private final ReedSolomonDecoder rsDecoder;

//public Decoder() {
//	rsDecoder = new ReedSolomonDecoder(GenericGF.DATA_MATRIX_FIELD_256);
//}

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
static ErrorStatus
CorrectErrors(ByteArray& codewordBytes, int numDataCodewords)
{
	// First read into an array of ints
	std::vector<int> codewordsInts(codewordBytes.begin(), codewordBytes.end());
	int numECCodewords = codewordBytes.length() - numDataCodewords;
	auto status = ReedSolomonDecoder(GenericGF::DataMatrixField256()).decode(codewordsInts, numECCodewords);
	if (StatusIsOK(status)) {
		// Copy back into array of bytes -- only need to worry about the bytes that were data
		// We don't care about errors in the error-correction codewords
		for (int i = 0; i < numDataCodewords; i++) {
			codewordBytes[i] = static_cast<uint8_t>(codewordsInts[i]);
		}
	}
	else if (StatusIsKindOf(status, ErrorStatus::ReedSolomonError)) {
		return ErrorStatus::ChecksumError;
	}
	return status;
}

DecoderResult
Decoder::Decode(const BitMatrix& bits)
{
	// Construct a parser and read version, error-correction level
	const Version* version = BitMatrixParser::ReadVersion(bits);
	if (version == nullptr) {
		return DecoderResult(ErrorStatus::FormatError);
	}
	
	// Read codewords
	ByteArray codewords;
	ErrorStatus status = BitMatrixParser::ReadCodewords(bits, codewords);
	if (StatusIsError(status)) {
		return DecoderResult(status);
	}

	// Separate into data blocks
	std::vector<DataBlock> dataBlocks;
	status = DataBlock::GetDataBlocks(codewords, *version, dataBlocks);
	if (StatusIsError(status)) {
		return DecoderResult(status);
	}

	// Count total number of data bytes
	int totalBytes = 0;
	for (const DataBlock& db : dataBlocks) {
		totalBytes += db.numDataCodewords();
	}
	ByteArray resultBytes(totalBytes);

	// Error-correct and copy data blocks together into a stream of bytes
	int dataBlocksCount = static_cast<int>(dataBlocks.size());
	for (int j = 0; j < dataBlocksCount; j++) {
		const auto& dataBlock = dataBlocks[j];
		ByteArray codewordBytes = dataBlock.codewords();
		int numDataCodewords = dataBlock.numDataCodewords();
		CorrectErrors(codewordBytes, numDataCodewords);
		for (int i = 0; i < numDataCodewords; i++) {
			// De-interlace data blocks.
			resultBytes[i * dataBlocksCount + j] = codewordBytes[i];
		}
	}

	// Decode the contents of that stream of bytes
	return DecodedBitStreamParser.decode(resultBytes);
}

} // DataMatrix
} // ZXing
