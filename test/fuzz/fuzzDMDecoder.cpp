#include <stdint.h>
#include <stddef.h>

#include "ByteArray.h"
#include "DecoderResult.h"

using namespace ZXing;

namespace ZXing::DataMatrix::DecodedBitStreamParser {
DecoderResult Decode(ByteArray&& bytes, const std::string& characterSet);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
		return 0;

	ByteArray ba;
	ba.insert(ba.begin(), data, data + size);
	try {
		DataMatrix::DecodedBitStreamParser::Decode(std::move(ba), "");
	} catch (...) {
	}

	return 0;
}
