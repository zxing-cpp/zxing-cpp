#pragma once
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

#include <memory>

namespace ZXing {

class DecoderResult;
class BitMatrix;
class DecodeHints;

namespace QRCode {


/**
* <p>The main class which implements QR Code decoding -- as opposed to locating and extracting
* the QR Code from an image.</p>
*/
class Decoder
{
public:
	/**
	* <p>Convenience method that can decode a QR Code represented as a 2D array of booleans.
	* "true" is taken to mean a black module.</p>
	*
	* @param image booleans representing white/black QR Code modules
	* @param hints decoding hints that should be used to influence decoding
	* @return text and bytes encoded within the QR Code
	* @throws FormatException if the QR Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	//DecoderResult decode(boolean[][] image, Map<DecodeHintType, ? > hints)
	//	throws ChecksumException, FormatException{
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

	//public DecoderResult decode(BitMatrix bits) throws ChecksumException, FormatException{
	//	return decode(bits, null);
	//}

	/**
	* <p>Decodes a QR Code represented as a {@link BitMatrix}. A 1 or "true" is taken to mean a black module.</p>
	*
	* @param bits booleans representing white/black QR Code modules
	* @param hints decoding hints that should be used to influence decoding
	* @return text and bytes encoded within the QR Code
	* @throws FormatException if the QR Code cannot be decoded
	* @throws ChecksumException if error correction fails
	*/
	static std::shared_ptr<DecoderResult> Decode(const BitMatrix& bits, const DecodeHints* hints = nullptr);

	//private DecoderResult decode(BitMatrixParser parser, Map<DecodeHintType, ? > hints)
	//	throws FormatException, ChecksumException{
	//	Version version = parser.readVersion();
	//ErrorCorrectionLevel ecLevel = parser.readFormatInformation().getErrorCorrectionLevel();

	//// Read codewords
	//byte[] codewords = parser.readCodewords();
	//// Separate into data blocks
	//DataBlock[] dataBlocks = DataBlock.getDataBlocks(codewords, version, ecLevel);

	//// Count total number of data bytes
	//int totalBytes = 0;
	//for (DataBlock dataBlock : dataBlocks) {
	//	totalBytes += dataBlock.getNumDataCodewords();
	//}
	//byte[] resultBytes = new byte[totalBytes];
	//int resultOffset = 0;

	//// Error-correct and copy data blocks together into a stream of bytes
	//for (DataBlock dataBlock : dataBlocks) {
	//	byte[] codewordBytes = dataBlock.getCodewords();
	//	int numDataCodewords = dataBlock.getNumDataCodewords();
	//	correctErrors(codewordBytes, numDataCodewords);
	//	for (int i = 0; i < numDataCodewords; i++) {
	//		resultBytes[resultOffset++] = codewordBytes[i];
	//	}
	//}

	//// Decode the contents of that stream of bytes
	//return DecodedBitStreamParser.decode(resultBytes, version, ecLevel, hints);
	//}

};

} // QRCode
} // ZXing
