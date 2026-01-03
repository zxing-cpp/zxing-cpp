/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "Barcode.h"

#include "DecoderResult.h"
#include "DetectorResult.h"
#include "JSON.h"
#include "ZXAlgorithms.h"

#include "BitMatrix.h"

#include <cmath>
#include <list>
#include <map>
#include <utility>

#ifdef ZXING_USE_ZINT
#include <zint.h>
void zint_symbol_deleter::operator()(zint_symbol* p) const noexcept
{
	ZBarcode_Delete(p);
}
#else
struct zint_symbol {};
#endif

namespace ZXing {

struct Barcode::Data
{
	Content content;
	Error error;
	Position position;
	BarcodeFormat format = BarcodeFormat::None;
	std::string extra;
	StructuredAppendInfo sai;
	ReaderOptions readerOpts;
	BitMatrix symbol;
	std::shared_ptr<zint_symbol> zint;
	char ecLevel[4] = {};
	char version[4] = {};
	int lineCount = 0;
	bool isMirrored = false;
	bool isInverted = false;

#ifndef __cpp_aggregate_paren_init // MSVC 17.14
	Data() = default;
	Data(Content&& c, Error&& e, Position&& p, BarcodeFormat f, std::string ex)
		: content(std::move(c)), error(std::move(e)), position(std::move(p)), format(f), extra(std::move(ex))
	{}
#endif
};

Barcode::Barcode() : d(std::make_shared<Data>()) {}

Barcode::Barcode(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, SymbologyIdentifier si, Error error,
				 std::string extra)
	: d(std::make_shared<Data>(Content({ByteArray(text)}, si), std::move(error), Position(Line(y, xStart, xStop)), format, std::move(extra)))
{}

Barcode::Barcode(DecoderResult&& decodeResult, DetectorResult&& detectorResult, BarcodeFormat format)
	: d(std::make_shared<Data>(std::move(decodeResult).content(), std::move(decodeResult).error(),
							   std::move(detectorResult).position(), format, std::move(decodeResult).json()))
{
	decodeResult.addExtra(BarcodeExtra::ReaderInit, decodeResult.readerInit());
	d->extra = std::move(decodeResult).json();
	d->sai = decodeResult.structuredAppend();
	d->symbol = std::move(detectorResult).bits();
	d->lineCount = decodeResult.lineCount();
	d->isMirrored = decodeResult.isMirrored();

	// the BitMatrix stores 'black'/foreground as 0xFF and 'white'/background as 0, but we
	// want the ImageView returned by symbol() to be a standard luminance image (black == 0)
	d->symbol.flipAll();

	if (decodeResult.versionNumber())
		snprintf(d->version, 4, "%d", decodeResult.versionNumber());
	snprintf(d->ecLevel, 4, "%s", decodeResult.ecLevel().data());
}

bool Barcode::isValid() const
{
	return format() != BarcodeFormat::None && !d->content.bytes.empty() && !error();
}

const Error& Barcode::error() const
{
	return d->error;
}

BarcodeFormat Barcode::format() const
{
	return d->format;
}

const Position& Barcode::position() const
{
	return d->position;
}

void Barcode::setPosition(Position pos)
{
	d->position = pos;
}

const std::vector<uint8_t>& Barcode::bytes() const
{
	return d->content.bytes;
}

std::vector<uint8_t> Barcode::bytesECI() const
{
	return d->content.bytesECI();
}

std::string Barcode::text(TextMode mode) const
{
	return d->content.text(mode);
}

std::string Barcode::text() const
{
	return text(d->readerOpts.textMode());
}

std::string Barcode::ecLevel() const
{
	return d->ecLevel;
}

ContentType Barcode::contentType() const
{
	return d->content.type();
}

bool Barcode::hasECI() const
{
	return d->content.hasECI;
}

int Barcode::orientation() const
{
	constexpr auto std_numbers_pi_v = 3.14159265358979323846; // TODO: c++20 <numbers>
	return narrow_cast<int>(std::lround(d->position.orientation() * 180 / std_numbers_pi_v));
}

bool Barcode::isMirrored() const
{
	return d->isMirrored;
}

bool Barcode::isInverted() const
{
	return d->isInverted;
}

std::string Barcode::symbologyIdentifier() const
{
	return d->content.symbology.toString();
}

int Barcode::sequenceSize() const
{
	return d->sai.count;
}

int Barcode::sequenceIndex() const
{
	return d->sai.index;
}

std::string Barcode::sequenceId() const
{
	return d->sai.id;
}

std::string Barcode::version() const
{
	return d->version;
}

int Barcode::lineCount() const
{
	return d->lineCount;
}

Barcode& Barcode::setReaderOptions(const ReaderOptions& opts)
{
	if (opts.characterSet() != CharacterSet::Unknown)
		d->content.defaultCharset = opts.characterSet();
	d->readerOpts = opts;
	return *this;
}

void Barcode::setIsInverted(bool v)
{
	d->isInverted = v;
}

void Barcode::incrementLineCount()
{
	++d->lineCount;
}

const BitMatrix& Barcode::symbolMatrix() const
{
	return d->symbol;
}

ImageView Barcode::symbol() const
{
	return !d->symbol.empty() ? ImageView{d->symbol.row(0).begin(), d->symbol.width(), d->symbol.height(), ImageFormat::Lum}
							  : ImageView{};
}

#ifdef ZXING_USE_ZINT
void Barcode::zint(unique_zint_symbol&& z)
{
	d->zint = std::shared_ptr(std::move(z));
}

zint_symbol* Barcode::zint() const
{
	return d->zint.get();
}
#endif

std::string Barcode::extra(std::string_view key) const
{
	if (key == "ALL") {
		if (format() == BarcodeFormat::None)
			return {};
		auto res =
			StrCat("{", JsonProp("Text", text(TextMode::Plain)), JsonProp("HRI", text(TextMode::HRI)),
				   JsonProp("TextECI", text(TextMode::ECI)), JsonProp("Bytes", text(TextMode::Hex)),
				   JsonProp("Identifier", symbologyIdentifier()), JsonProp("Format", ToString(format())),
				   JsonProp("ContentType", isValid() ? ToString(contentType()) : ""), JsonProp("Position", ToString(position())),
				   JsonProp("HasECI", hasECI()), JsonProp("IsMirrored", isMirrored()), JsonProp("IsInverted", isInverted()), d->extra,
				   JsonProp("Error", ToString(error())));
		res.back() = '}';
		return res;
	}
	return d->extra.empty() ? "" : key.empty() ? StrCat("{", d->extra.substr(0, d->extra.size() - 1), "}") : std::string(JsonGetStr(d->extra, key));
}

bool Barcode::operator==(const Barcode& o) const
{
	if (format() != o.format())
		return false;

	// handle MatrixCodes first
	if (!IsLinearBarcode(format())) {
		if (bytes() != o.bytes() && isValid() && o.isValid())
			return false;

		// check for equal position if both are valid with equal bytes or at least one is in error
		return IsInside(Center(o.position()), position());
	}

	if (bytes() != o.bytes() || error() != o.error() || orientation() != o.orientation())
		return false;

	if (lineCount() > 1 && o.lineCount() > 1)
		return HaveIntersectingBoundingBoxes(o.position(), position());

	// the following code is only meant for this or other lineCount == 1
	assert(lineCount() == 1 || o.lineCount() == 1);

	// sl == single line, ml = multi line
	const auto& sl = lineCount() == 1 ? *this : o;
	const auto& ml = lineCount() == 1 ? o : *this;

	// If one line is less than half the length of the other away from the
	// latter, we consider it to belong to the same symbol.
	// Additionally, both need to have roughly the same length (see #367).
	auto dTop = maxAbsComponent(ml.position().topLeft() - sl.position().topLeft());
	auto dBot = maxAbsComponent(ml.position().bottomLeft() - sl.position().topLeft());
	auto slLength = maxAbsComponent(sl.position().topLeft() - sl.position().bottomRight());
	bool isHorizontal = sl.position().topLeft().y == sl.position().bottomRight().y;
	// Measure the multi line length in the same direction as the single line one (not diagonaly)
	// to make sure overly tall symbols don't get segmented (see #769).
	auto mlLength = isHorizontal ? std::abs(ml.position().topLeft().x - ml.position().bottomRight().x)
								 : std::abs(ml.position().topLeft().y - ml.position().bottomRight().y);

	return std::min(dTop, dBot) < slLength / 2 && std::abs(slLength - mlLength) < slLength / 5;
}

Barcode MergeStructuredAppendSequence(const Barcodes& barcodes)
{
	if (barcodes.empty())
		return {};

	std::list<Barcode> allBarcodes(barcodes.begin(), barcodes.end());
	allBarcodes.sort([](const Barcode& r1, const Barcode& r2) { return r1.sequenceIndex() < r2.sequenceIndex(); });

	Barcode res = allBarcodes.front();
	for (auto i = std::next(allBarcodes.begin()); i != allBarcodes.end(); ++i)
		res.d->content.append(i->d->content);

	res.d->position = {};
	res.d->sai.index = -1;

	if (allBarcodes.back().sequenceSize() != Size(allBarcodes) ||
		!std::all_of(allBarcodes.begin(), allBarcodes.end(),
					 [&](Barcode& it) { return it.sequenceId() == allBarcodes.front().sequenceId(); }))
		res.d->error = FormatError("sequenceIDs not matching during structured append sequence merging");

	return res;
}

Barcodes MergeStructuredAppendSequences(const Barcodes& barcodes)
{
	std::map<std::string, Barcodes> sas;
	for (auto& barcode : barcodes) {
		if (barcode.isPartOfSequence())
			sas[barcode.sequenceId()].push_back(barcode);
	}

	Barcodes res;
	for (auto& [id, seq] : sas) {
		auto barcode = MergeStructuredAppendSequence(seq);
		if (barcode.isValid())
			res.push_back(std::move(barcode));
	}

	return res;
}

} // namespace ZXing
