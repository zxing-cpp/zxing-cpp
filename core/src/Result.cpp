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

#include <cmath>
#include <utility>

namespace ZXing {

Result::Result(std::wstring&& text, Position&& position, BarcodeFormat format, std::string&& symbologyIdentifier,
			   ByteArray&& rawBytes, StructuredAppendInfo&& sai, const bool readerInit, int lineCount)
	: _format(format),
	  _text(std::move(text)),
	  _position(std::move(position)),
	  _rawBytes(std::move(rawBytes)),
	  _symbologyIdentifier(std::move(symbologyIdentifier)),
	  _sai(std::move(sai)),
	  _readerInit(readerInit),
	  _lineCount(lineCount)
{
	_numBits = Size(_rawBytes) * 8;
}

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format,
			   std::string&& symbologyIdentifier, ByteArray&& rawBytes, const bool readerInit)
	: Result(TextDecoder::FromLatin1(text), Line(y, xStart, xStop), format, std::move(symbologyIdentifier),
			 std::move(rawBytes), {}, readerInit)
{}

Result::Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format)
	: _status(decodeResult.errorCode()),
	  _format(format),
	  _text(std::move(decodeResult).text()),
	  _position(std::move(position)),
	  _rawBytes(std::move(decodeResult).rawBytes()),
	  _numBits(decodeResult.numBits()),
	  _ecLevel(decodeResult.ecLevel()),
	  _symbologyIdentifier(decodeResult.symbologyIdentifier()),
	  _sai(decodeResult.structuredAppend()),
	  _isMirrored(decodeResult.isMirrored()),
	  _readerInit(decodeResult.readerInit())
{
	// TODO: add type opaque and code specific 'extra data'? (see DecoderResult::extra())
}

int Result::orientation() const
{
	constexpr auto std_numbers_pi_v = 3.14159265358979323846; // TODO: c++20 <numbers>
	return std::lround(_position.orientation() * 180 / std_numbers_pi_v);
}

bool Result::operator==(const Result& o) const
{
	if (format() != o.format() || text() != o.text())
		return false;

	if (BarcodeFormats(BarcodeFormat::TwoDCodes).testFlag(format()))
		return IsInside(Center(o.position()), position());

	// if one line is less than half the length of the other away from the
	// latter, we consider it to belong to the same symbol
	auto dTop = maxAbsComponent(o.position().topLeft() - position().topLeft());
	auto dBot = maxAbsComponent(o.position().bottomLeft() - position().topLeft());
	auto length = maxAbsComponent(position().topLeft() - position().bottomRight());

	return std::min(dTop, dBot) < length / 2;
}

} // ZXing
