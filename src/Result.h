#pragma once
#include "ZXString.h"
#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "ResultPoint.h"
#include "ResultMetadata.h"

#include <chrono>

namespace ZXing {

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*/
class Result
{
	String _text;
	ByteArray _rawBytes;
	std::vector<ResultPoint> _resultPoints;
	BarcodeFormat _format;
	ResultMetadata _metadata;
	std::chrono::steady_clock::time_point _timestamp;
};

}