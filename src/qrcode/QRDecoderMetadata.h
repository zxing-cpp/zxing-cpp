#pragma once
#include "ResultMetadata.h"
#include <algorithm>

namespace ZXing {

class ResultPoint;

namespace QRCode {

/**
* Meta-data container for QR Code decoding. Instances of this class may be used to convey information back to the
* decoding caller. Callers are expected to process this.
*
* @see com.google.zxing.common.DecoderResult#getOther()
*/

class DecoderMetadata : public ResultMetadata
{
	bool _mirrored;

public:
	explicit DecoderMetadata(bool mirrored) : _mirrored(mirrored) {}

	/**
	* @return true if the QR Code was mirrored.
	*/
	bool isMirrored() const {
		return _mirrored;
	}

	/**
	* Apply the result points' order correction due to mirroring.
	*
	* @param points Array of points to apply mirror correction to.
	*/
	template <typename Iter>
	void applyMirroredCorrection(Iter first, Iter end) const
	{
		using std::swap;
		if (!_mirrored)
			return;
		if (std::distance(first, end) < 3)
			return;
		swap(*first, *(first + 2));
		// No need to 'fix' top-left and alignment pattern.
	}
};

} // QRCode
} // ZXing
