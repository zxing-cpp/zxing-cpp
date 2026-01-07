/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Barcode.h"
#include "BitMatrix.h"
#include "Content.h"
#include "Error.h"
#include "ReaderOptions.h"
#include "StructuredAppend.h"

#include <memory>
#include <string>

#ifdef ZXING_USE_ZINT
extern "C" struct zint_symbol;
struct zint_symbol_deleter
{
	void operator()(zint_symbol* p) const noexcept;
};
using unique_zint_symbol = std::unique_ptr<zint_symbol, zint_symbol_deleter>;
#else
struct zint_symbol {};
using unique_zint_symbol = std::unique_ptr<zint_symbol>;
#endif

namespace ZXing {

struct BarcodeData
{
	Content content;
	Error error = {};
	Position position = {};
	BarcodeFormat format = BarcodeFormat::None;
	std::string extra = {};
	StructuredAppendInfo sai = {};
	ReaderOptions readerOpts = {};
	BitMatrix symbol = {};
#ifdef ZXING_USE_ZINT
	unique_zint_symbol zint = {};
#endif
	int lineCount = 0;
	bool isMirrored = false;
	bool isInverted = false;

#ifndef __cpp_aggregate_paren_init // MSVC 17.14 compatibility
	BarcodeData() = default;
	BarcodeData(Content&& c, Error&& e, Position&& p, BarcodeFormat f, std::string&& ex)
		: content(std::move(c)), error(std::move(e)), position(std::move(p)), format(f), extra(std::move(ex))
	{}
#endif

	bool operator==(const BarcodeData& other) const;

	inline bool isValid() const { return format != BarcodeFormat::None && !content.bytes.empty() && !error; }

	inline int orientation() const
	{
		constexpr auto std_numbers_pi_v = 3.14159265358979323846; // TODO: c++20 <numbers>
		return narrow_cast<int>(std::lround(position.orientation() * 180 / std_numbers_pi_v));
	}
};

inline BarcodeData LinearBarcode(BarcodeFormat format, const std::string& text, int y, int xStart, int xStop, SymbologyIdentifier si,
                                 Error error = {}, std::string extra = {})
{
	return {.content = Content(ByteArray(text), si),
			.error = std::move(error),
			.position = Line(y, xStart, xStop),
			.format = format,
			.extra = std::move(extra)};
}

} // namespace ZXing