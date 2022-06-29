/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Result.h"

namespace ZXing {

class BinaryBitmap;
class DecodeHints;

class Reader
{
protected:
	const DecodeHints& _hints;

public:
	explicit Reader(const DecodeHints& hints) : _hints(hints) {}
	virtual ~Reader() = default;

	virtual Result decode(const BinaryBitmap& image) const = 0;

	// WARNING: this API is experimental and may change/disappear
	virtual Results decode(const BinaryBitmap& image, [[maybe_unused]] int maxSymbols) const {
		auto res = decode(image);
		return res.isValid() ? Results{std::move(res)} : Results{};
	}
};

} // ZXing
