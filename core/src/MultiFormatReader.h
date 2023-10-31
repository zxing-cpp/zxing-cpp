/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Result.h"

#include <vector>
#include <memory>

namespace ZXing {

class Result;
class Reader;
class BinaryBitmap;
class DecodeHints;

/**
* MultiFormatReader is a convenience class and the main entry point into the library for most uses.
* By default it attempts to decode all barcode formats that the library supports. Optionally, you
* can provide a hints object to request different behavior, for example only decoding QR codes.
*
* @author Sean Owen
* @author dswitkin@google.com (Daniel Switkin)
*/
class MultiFormatReader
{
public:
	explicit MultiFormatReader(const DecodeHints& hints);
	explicit MultiFormatReader(DecodeHints&& hints) = delete;
	~MultiFormatReader();

	Result read(const BinaryBitmap& image) const;

	// WARNING: this API is experimental and may change/disappear
	Results readMultiple(const BinaryBitmap& image, int maxSymbols = 0xFF) const;

private:
	std::vector<std::unique_ptr<Reader>> _readers;
	const DecodeHints& _hints;
};

} // ZXing
