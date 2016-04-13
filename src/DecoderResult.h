#pragma once
#include "ByteArray.h"
#include "ZXString.h"

#include <memory>

namespace ZXing {

class ResultMetadata;

/**
* <p>Encapsulates the result of decoding a matrix of bits. This typically
* applies to 2D barcode formats. For now it contains the raw bytes obtained,
* as well as a String interpretation of those bytes, if applicable.</p>
*/
class DecoderResult
{
	ByteArray _rawBytes;
	String _text;
	std::shared_ptr<ResultMetadata> _metadata;

public:
	~DecoderResult();

	void setMetadata(const std::shared_ptr<ResultMetadata>& m);
};

} // ZXing