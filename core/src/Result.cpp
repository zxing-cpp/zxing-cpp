/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "Result.h"

#include "DecoderResult.h"
#include "TextDecoder.h"
#include "ZXAlgorithms.h"

#include <cmath>
#include <list>
#include <map>
#include <utility>

namespace ZXing {

Result::Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, SymbologyIdentifier si, Error error, bool readerInit)
	: _content({ByteArray(text)}, si),
	  _error(error),
	  _position(Line(y, xStart, xStop)),
	  _format(format),
	  _readerInit(readerInit)
{}

Result::Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format)
	: _content(std::move(decodeResult).content()),
	  _error(std::move(decodeResult).error()),
	  _position(std::move(position)),
	  _sai(decodeResult.structuredAppend()),
	  _format(format),
	  _lineCount(decodeResult.lineCount()),
	  _isMirrored(decodeResult.isMirrored()),
	  _readerInit(decodeResult.readerInit())
{
	if (decodeResult.versionNumber())
		snprintf(_version, 4, "%d", decodeResult.versionNumber());
	snprintf(_ecLevel, 4, "%s", decodeResult.ecLevel().data());

	// TODO: add type opaque and code specific 'extra data'? (see DecoderResult::extra())
}

bool Result::isValid() const
{
	return format() != BarcodeFormat::None && _content.symbology.code != 0 && !error();
}

const ByteArray& Result::bytes() const
{
	return _content.bytes;
}

ByteArray Result::bytesECI() const
{
	return _content.bytesECI();
}

std::string Result::text(TextMode mode) const
{
	return _content.text(mode);
}

std::string Result::text() const
{
	return text(_decodeHints.textMode());
}

std::string Result::ecLevel() const
{
	return _ecLevel;
}

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

std::string Result::version() const
{
	return _version;
}

Result& Result::setDecodeHints(DecodeHints hints)
{
	if (hints.characterSet() != CharacterSet::Unknown)
		_content.defaultCharset = hints.characterSet();
	_decodeHints = hints;
	return *this;
}

bool Result::operator==(const Result& o) const
{
	if (format() != o.format() || bytes() != o.bytes() || error() != o.error())
		return false;

	if (BarcodeFormats(BarcodeFormat::MatrixCodes).testFlag(format()))
		return IsInside(Center(o.position()), position());

	if (orientation() != o.orientation())
		return false;

	if (lineCount() > 1 && o.lineCount() > 1)
		return IsInside(Center(o.position()), position());

	// the following code is only meant for this->lineCount == 1
	assert(lineCount() == 1);

	// if one line is less than half the length of the other away from the
	// latter, we consider it to belong to the same symbol. additionally, both need to have
	// roughly the same length (see #367)
	auto dTop = maxAbsComponent(o.position().topLeft() - position().topLeft());
	auto dBot = maxAbsComponent(o.position().bottomLeft() - position().topLeft());
	auto length = maxAbsComponent(position().topLeft() - position().bottomRight());
	auto dLength = std::abs(length - maxAbsComponent(o.position().topLeft() - o.position().bottomRight()));

	return std::min(dTop, dBot) < length / 2 && dLength < length / 5;
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
