/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "Result.h"

#include "DecoderResult.h"
#include "TextDecoder.h"

#include <cmath>
#include <utility>

namespace ZXing {

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format,
			   std::string&& symbologyIdentifier, ByteArray&& rawBytes, const bool readerInit)
	:
	  _format(format),
	  _content({ByteArray(text), ECI::ISO8859_1}),
	  _text(TextDecoder::FromLatin1(text)),
	  _position(Line(y, xStart, xStop)),
	  _rawBytes(std::move(rawBytes)),
	  _numBits(Size(_rawBytes) * 8),
	  _symbologyIdentifier(std::move(symbologyIdentifier)),
	  _readerInit(readerInit),
	  _lineCount(0)
{}

Result::Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format)
	: _status(decodeResult.errorCode()),
	  _format(format),
	  _content(std::move(decodeResult).content()),
	  _text(std::move(decodeResult).text()),
	  _position(std::move(position)),
	  _rawBytes(std::move(decodeResult).rawBytes()),
	  _numBits(decodeResult.numBits()),
	  _ecLevel(TextDecoder::FromLatin1(decodeResult.ecLevel())),
	  _symbologyIdentifier(decodeResult.symbologyIdentifier()),
	  _sai(decodeResult.structuredAppend()),
	  _isMirrored(decodeResult.isMirrored()),
	  _readerInit(decodeResult.readerInit()),
	  _lineCount(decodeResult.lineCount())
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
