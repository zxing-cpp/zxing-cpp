#pragma once
#include "qrcode/QRECB.h"
#include "qrcode/ErrorCorrectionLevel.h"

#include <initializer_list>
#include <vector>

namespace ZXing {

class BitMatrix;

namespace QRCode {

/**
* See ISO 18004:2006 Annex D
*/
class Version
{
public:
	int versionNumber() const {
		return _versionNumber;
	}

	const std::vector<int>& alignmentPatternCenters() const {
		return _alignmentPatternCenters;
	}
	
	int totalCodewords() const {
		return _totalCodewords;
	}
	
	int dimensionForVersion() const {
		return 17 + 4 * _versionNumber;
	}
	
	const ECBlocks & ecBlocksForLevel(ErrorCorrectionLevel ecLevel) const {
		return _ecBlocks[(int)ecLevel];
	}

	BitMatrix buildFunctionPattern() const;
	
	/**
	* <p>Deduces version information purely from QR Code dimensions.</p>
	*
	* @param dimension dimension in modules
	* @return Version for a QR Code of that dimension
	* @throws FormatException if dimension is not 1 mod 4
	*/
	static const Version* ProvisionalVersionForDimension(int dimension);
	
	static const Version* VersionForNumber(int versionNumber);

	static const Version* Version::DecodeVersionInformation(int versionBits);


	
private:
	int _versionNumber;
	std::vector<int> _alignmentPatternCenters;
	std::array<ECBlocks, 4> _ecBlocks;
	int _totalCodewords;

	Version(int versionNumber, std::initializer_list<int> alignmentPatternCenters, const std::array<ECBlocks, 4> &ecBlocks);
	static const Version* AllVersions();
};

} // QRCode
} // ZXing
