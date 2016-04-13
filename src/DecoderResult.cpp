#include "DecoderResult.h"
#include "ResultMetadata.h"

namespace ZXing {

DecoderResult::~DecoderResult()
{
}

void
DecoderResult::setMetadata(const std::shared_ptr<ResultMetadata>& m)
{
	_metadata = m;
}

} // ZXing