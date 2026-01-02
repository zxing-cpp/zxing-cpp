/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Barcode.h"
#include "ImageView.h"

#include <memory>

namespace ZXing {

class WriterOptions
{
	struct Data;

	std::unique_ptr<Data> d;

public:
	WriterOptions();
	~WriterOptions();
	WriterOptions(WriterOptions&&);
	WriterOptions& operator=(WriterOptions&&);

#define ZX_PROPERTY(TYPE, NAME) \
	TYPE NAME() const noexcept; \
	WriterOptions& NAME(TYPE v)&; \
	WriterOptions&& NAME(TYPE v)&&;

	ZX_PROPERTY(int, scale)
	ZX_PROPERTY(int, sizeHint)
	ZX_PROPERTY(int, rotate)
	ZX_PROPERTY(bool, invert)
	ZX_PROPERTY(bool, addHRT)
	ZX_PROPERTY(bool, addQuietZones)

#undef ZX_PROPERTY
};


/**
 * Write barcode symbol to SVG
 *
 * @param barcode  Barcode to write
 * @param options  WriterOptions to parameterize rendering
 * @return std::string  SVG representation of barcode symbol
 */
std::string WriteBarcodeToSVG(const Barcode& barcode, const WriterOptions& options = {});

/**
 * Write barcode symbol to a utf8 string using graphical characters (e.g. '▀')
 *
 * @param barcode  Barcode to write
 * @param options  WriterOptions to parameterize rendering
 * @return std::string  Utf8 string representation of barcode symbol
 */
std::string WriteBarcodeToUtf8(const Barcode& barcode, const WriterOptions& options = {});

/**
 * Write barcode symbol to Image (Bitmap)
 *
 * @param barcode  Barcode to write
 * @param options  WriterOptions to parameterize rendering
 * @return Image  Bitmap reprentation of barcode symbol
 */
Image WriteBarcodeToImage(const Barcode& barcode, const WriterOptions& options = {});

} // ZXing
