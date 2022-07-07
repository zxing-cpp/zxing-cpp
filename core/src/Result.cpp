/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "Result.h"

#include "DecoderResult.h"
#include "TextDecoder.h"
#include "TextUtfEncoding.h"
#include "ZXAlgorithms.h"

#include <cmath>
#include <list>
#include <map>
#include <utility>

namespace ZXing {

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, SymbologyIdentifier si, Error error,
			   ByteArray&& rawBytes, bool readerInit, const std::string& ai)
	: _format(format),
	  _content({ByteArray(text)}, si, ai),
	  _error(error),
	  _position(Line(y, xStart, xStop)),
	  _rawBytes(std::move(rawBytes)),
	  _numBits(Size(_rawBytes) * 8),
	  _readerInit(readerInit),
	  _lineCount(0)
{}

Result::Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format)
	: _format(decodeResult.content().symbology.code == 0 ? BarcodeFormat::None : format),
	  _content(std::move(decodeResult).content()),
	  _error(std::move(decodeResult).error()),
	  _position(std::move(position)),
	  _rawBytes(std::move(decodeResult).rawBytes()),
	  _numBits(decodeResult.numBits()),
	  _ecLevel(decodeResult.ecLevel()),
	  _sai(decodeResult.structuredAppend()),
	  _isMirrored(decodeResult.isMirrored()),
	  _readerInit(decodeResult.readerInit()),
	  _lineCount(decodeResult.lineCount())
{
	// TODO: add type opaque and code specific 'extra data'? (see DecoderResult::extra())
}

DecodeStatus Result::status() const
{
	switch(_error.type()) {
	case Error::Format : return DecodeStatus::FormatError;
	case Error::Checksum : return DecodeStatus::ChecksumError;
	default: ;
	}

	return format() == BarcodeFormat::None ? DecodeStatus::NotFound : DecodeStatus::NoError;
}

const ByteArray& Result::bytes() const
{
	return _content.bytes;
}

ByteArray Result::bytesECI() const
{
	return _content.bytesECI();
}

std::string Result::utf8() const
{
	return _content.utf8();
}

std::wstring Result::utf16() const
{
	return _content.utf16();
}

#if 0
std::string Result::utf8ECI() const
{
	return _content.text(TextMode::Utf8ECI);
}
#endif

ContentType Result::contentType() const
{
	return _content.type();
}

bool Result::hasECI() const
{
	return _content.hasECI;
}

int Result::orientation() const
{
	constexpr auto std_numbers_pi_v = 3.14159265358979323846; // TODO: c++20 <numbers>
	return narrow_cast<int>(std::lround(_position.orientation() * 180 / std_numbers_pi_v));
}

std::string Result::symbologyIdentifier() const
{
	return _content.symbology.toString();
}

int Result::sequenceSize() const
{
	return _sai.count;
}

int Result::sequenceIndex() const
{
	return _sai.index;
}

std::string Result::sequenceId() const
{
	return _sai.id;
}

Result& Result::setCharacterSet(const std::string& defaultCS)
{
	if (!defaultCS.empty())
		_content.defaultCharset = defaultCS;
	return *this;
}

bool Result::operator==(const Result& o) const
{
	// two symbols may be considered the same if at least one of them has an error
	if (!(format() == o.format() && (bytes() == o.bytes() || error() || o.error())))
		return false;

	if (BarcodeFormats(BarcodeFormat::MatrixCodes).testFlag(format()))
		return IsInside(Center(o.position()), position());

	// linear symbology comparisons only implemented for this->lineCount == 1
	assert(lineCount() == 1);

	// if one line is less than half the length of the other away from the
	// latter, we consider it to belong to the same symbol
	auto dTop = maxAbsComponent(o.position().topLeft() - position().topLeft());
	auto dBot = maxAbsComponent(o.position().bottomLeft() - position().topLeft());
	auto length = maxAbsComponent(position().topLeft() - position().bottomRight());

	return std::min(dTop, dBot) < length / 2;
}

Result MergeStructuredAppendSequence(const Results& results)
{
	if (results.empty())
		return {};

	std::list<Result> allResults(results.begin(), results.end());
	allResults.sort([](const Result& r1, const Result& r2) { return r1.sequenceIndex() < r2.sequenceIndex(); });

	Result res = allResults.front();
	for (auto i = std::next(allResults.begin()); i != allResults.end(); ++i)
		res._content.append(i->_content);

	res._position = {};
	res._sai.index = -1;

	if (allResults.back().sequenceSize() != Size(allResults) ||
		!std::all_of(allResults.begin(), allResults.end(),
					 [&](Result& it) { return it.sequenceId() == allResults.front().sequenceId(); }))
		res._error = FormatError("sequenceIDs not matching during structured append sequence merging");

	return res;
}

Results MergeStructuredAppendSequences(const Results& results)
{
	std::map<std::string, Results> sas;
	for (auto& res : results) {
		if (res.isPartOfSequence())
			sas[res.sequenceId()].push_back(res);
	}

	Results saiResults;
	for (auto& [id, seq] : sas) {
		auto res = MergeStructuredAppendSequence(seq);
		if (res.isValid())
			saiResults.push_back(std::move(res));
	}

	return saiResults;
}

} // ZXing
