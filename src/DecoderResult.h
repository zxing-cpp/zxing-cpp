#pragma once
#include "utils/ByteArray.h"
#include "utils/ZXString.h"

namespace ZXing {

/**
* <p>Encapsulates the result of decoding a matrix of bits. This typically
* applies to 2D barcode formats. For now it contains the raw bytes obtained,
* as well as a String interpretation of those bytes, if applicable.</p>
*/
class DecoderResult
{
	ByteArray _rawBytes;
	String _text;
};

} // ZXing