#pragma once

namespace ZXing {

class BitMatrix;
class ByteArray;

namespace QRCode {

class Version;
class FormatInformation;

class BitMatrixParser
{
public:
	/**
	* @param bitMatrix {@link BitMatrix} to parse
	* return false if dimension is not >= 21 and 1 mod 4
	*/
	static bool ParseVersionInfo(const BitMatrix& bitMatrix, bool mirrored, const Version*& parsedVersion, FormatInformation& parsedFormatInfo);

	/**
	* <p>Reads the bits in the {@link BitMatrix} representing the finder pattern in the
	* correct order in order to reconstruct the codewords bytes contained within the
	* QR Code.</p>
	*
	* @return bytes encoded within the QR Code
	* or empty array if the exact number of bytes expected is not read
	*/
	static ByteArray ReadCodewords(const BitMatrix& bitMatrix, const Version& version);
};

} // QRCode
} // ZXing