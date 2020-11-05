#include <stdint.h>
#include <stddef.h>

#include "BitArray.h"
#include "oned/rss/ODRSSExpandedBinaryDecoder.h"

using namespace ZXing;
using namespace ZXing::OneD::RSS;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	if (size < 2)
		return 0;

	BitArray bits;
	for (size_t i = 0; i < size; ++i)
		bits.appendBits(data[i], 8);

	ExpandedBinaryDecoder::Decode(bits);

	return 0;
}
