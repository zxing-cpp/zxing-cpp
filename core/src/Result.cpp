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
#include "TextDecoder.h"

namespace ZXing {

Result::Result(std::wstring&& text, std::vector<ResultPoint>&& resultPoints, BarcodeFormat format, ByteArray&& rawBytes)
    : _text(std::move(text)),
      _rawBytes(std::move(rawBytes)),
      _resultPoints(std::move(resultPoints)),
      _format(format)
{
	_numBits = static_cast<int>(_rawBytes.size()) * 8;
}

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, ByteArray&& rawBytes)
    : Result(TextDecoder::FromLatin1(text), {ResultPoint(xStart, y), ResultPoint(xStop, y)}, format, std::move(rawBytes))
{}

Result::Result(DecoderResult&& decodeResult, std::vector<ResultPoint>&& resultPoints, BarcodeFormat format)
    : _status(decodeResult.errorCode()),
      _text(std::move(decodeResult).text()),
      _rawBytes(std::move(decodeResult).rawBytes()),
      _numBits(decodeResult.numBits()),
      _resultPoints(std::move(resultPoints)),
      _format(format)
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

void
Result::addResultPoints(const std::vector<ResultPoint>& points)
{
	_resultPoints.insert(resultPoints().end(), points.begin(), points.end());
}

} // ZXing
