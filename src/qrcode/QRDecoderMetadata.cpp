#include "qrcode/QRDecoderMetadata.h"

namespace ZXing {
namespace QRCode {

/**
* Apply the result points' order correction due to mirroring.
*
* @param points Array of points to apply mirror correction to.
*/
void
DecoderMetaData::ApplyMirroredCorrection(ResultPoint points) {
	if (!mirrored || points == null || points.length < 3) {
		return;
	}
	ResultPoint bottomLeft = points[0];
	points[0] = points[2];
	points[2] = bottomLeft;
	// No need to 'fix' top-left and alignment pattern.
}

} // QRCode
} // ZXing
