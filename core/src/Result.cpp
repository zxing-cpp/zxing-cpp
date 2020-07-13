/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Result.h"
#include "DecoderResult.h"
#include "TextDecoder.h"
#include "ZXNumeric.h"

#include <cmath>
#include <utility>

namespace ZXing {

Result::Result(std::wstring&& text, Position&& position, BarcodeFormat format, ByteArray&& rawBytes)
	: _format(format), _text(std::move(text)), _position(std::move(position)), _rawBytes(std::move(rawBytes))
{
	_numBits = Size(_rawBytes) * 8;
}

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, ByteArray&& rawBytes)
	: Result(TextDecoder::FromLatin1(text), Line(y, xStart, xStop), format, std::move(rawBytes))
{}

Result::Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format)
	: _status(decodeResult.errorCode()), _format(format), _text(std::move(decodeResult).text()),
	  _position(std::move(position)), _rawBytes(std::move(decodeResult).rawBytes()), _numBits(decodeResult.numBits())
{
	if (!isValid())
		return;

	//TODO: change ResultMetadata::put interface, so we can move from decodeResult?
	const auto& byteSegments = decodeResult.byteSegments();
	if (!byteSegments.empty()) {
		metadata().put(ResultMetadata::BYTE_SEGMENTS, byteSegments);
	}
	const auto& ecLevel = decodeResult.ecLevel();
	if (!ecLevel.empty()) {
		metadata().put(ResultMetadata::ERROR_CORRECTION_LEVEL, ecLevel);
	}
	if (decodeResult.hasStructuredAppend()) {
		metadata().put(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, decodeResult.structuredAppendSequenceNumber());
		metadata().put(ResultMetadata::STRUCTURED_APPEND_CODE_COUNT, decodeResult.structuredAppendCodeCount());
		metadata().put(ResultMetadata::STRUCTURED_APPEND_PARITY, decodeResult.structuredAppendParity());
	}
	//TODO: what about the other optional data in DecoderResult?
}

int Result::orientation() const
{
	return std::lround(_position.orientation() * kDegPerRad);
}

} // ZXing
