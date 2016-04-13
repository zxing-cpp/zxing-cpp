#include "qrcode/QRDecoder.h"
#include "qrcode/QRBitMatrixParser.h"
#include "qrcode/QRVersion.h"
#include "qrcode/FormatInformation.h"
#include "qrcode/QRDecoderMetadata.h"
#include "qrcode/QRDataMask.h"
#include "qrcode/QRDataBlock.h"
#include "DecoderResult.h"
#include "BitMatrix.h"
#include "ReedSolomonDecoder.h"
#include "GenericGF.h"

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
	int numCodewords = codewordBytes.length;
	// First read into an array of ints
	std::vector<int> codewordsInts(numCodewords);
	for (int i = 0; i < numCodewords; i++) {
		codewordsInts[i] = codewordBytes[i] & 0xFF;
	}
	int numECCodewords = codewordBytes.length - numDataCodewords;
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

std::shared_ptr<DecoderResult> DoDecode(const BitMatrix& bits, const Version& version, const FormatInformation& formatInfo, const DecodeHints* hints)
{
	auto ecLevel = formatInfo.errorCorrectionLevel();

	// Read codewords
	auto codewords = BitMatrixParser::ReadCodewords(bits, version);
	// Separate into data blocks
	std::vector<DataBlock> dataBlocks;
	if (!DataBlock::GetDataBlocks(codewords, version, ecLevel, dataBlocks))
		return nullptr;

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
			return nullptr;

		for (int i = 0; i < numDataCodewords; i++) {
			resultBytes[resultOffset++] = codewordBytes[i];
		}
	}

	// Decode the contents of that stream of bytes
	return DecodedBitStreamParser.decode(resultBytes, version, ecLevel, hints);

}

void ReMask(BitMatrix& bitMatrix, const FormatInformation& formatInfo)
{
	int dimension = bitMatrix.height();
	DataMask(formatInfo.dataMask()).unmaskBitMatrix(bitMatrix, dimension);
}

} // anonymous


std::shared_ptr<DecoderResult>
Decoder::Decode(const BitMatrix& bits_, const DecodeHints* hints)
{
	BitMatrix bits = bits_;

	// Construct a parser and read version, error-correction level
	const Version* version;
	FormatInformation formatInfo;
	if (BitMatrixParser::ParseVersionInfo(bits, false, version, formatInfo))
	{
		ReMask(bits, formatInfo);
		if (auto result = DoDecode(bits, *version, formatInfo, hints))
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
		if (auto result = DoDecode(bits, *version, formatInfo, hints))
		{
			result->setMetadata(std::make_shared<DecoderMetadata>(true));
			return result;
		}
	}
	return nullptr;
}

} // QRCode
} // ZXing