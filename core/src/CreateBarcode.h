/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Barcode.h"

#include <memory>
#include <optional>
#include <string_view>

extern "C" struct zint_symbol;

namespace ZXing {

/**
 * @brief Configuration options for barcode creation.
 *
 * This class encapsulates all the parameters needed to create a barcode with
 * specific format and settings.
 *
 * The options property is a string that can contain multiple key-value pairs
 * separated by commas. Each key-value pair configures a specific aspect of the
 * barcode generation process and are dependent on the used BarcodeFormat.
 * Boolean properties are interpreted as true if only the property name is present.
 * Keys are case-insensitive. Passing a serialized JSON object is also supported.
 *
 * For a list of all supported options, see the list of read-only properties below.
 *
 * ```c++
 * // example of creating a QR code with 30% error correction level and GS1 mode enabled
 * auto opts = CreatorOptions(BarcodeFormat::QRCode, "ecLevel=30%, gs1");
 * ```
 */
class CreatorOptions
{
	struct Data;

	std::unique_ptr<Data> d;

	friend Barcode CreateBarcode(const void* data, int size, int mode, const CreatorOptions& options);

public:
	CreatorOptions(BarcodeFormat format, std::string options = {});
	~CreatorOptions();
	CreatorOptions(CreatorOptions&&) noexcept;
	CreatorOptions& operator=(CreatorOptions&&) noexcept;

	zint_symbol* zint() const;

#define ZX_PROPERTY(TYPE, NAME) \
	const TYPE& NAME() const noexcept; \
	CreatorOptions& NAME(TYPE v)&; \
	CreatorOptions&& NAME(TYPE v)&&;

	ZX_PROPERTY(BarcodeFormat, format)
	ZX_PROPERTY(std::string, options)

#undef ZX_PROPERTY

#define ZX_RO_PROPERTY(TYPE, NAME) \
	std::optional<TYPE> NAME() const noexcept;

	/// EcLevel, e.g. "30%", see also libzint docs for supported values per symbology
	ZX_RO_PROPERTY(std::string, ecLevel);

	/// Specify ECI designator to use (e.g. "UTF-8" or "26" for UTF-8), see also libzint docs (most 2D symbologies)
	ZX_RO_PROPERTY(std::string, eci);

	/// GS1 mode (most 2D symbologies + Code128)
	ZX_RO_PROPERTY(bool, gs1);

	/// Set the "reader init" flag (most 2D symbologies)
	ZX_RO_PROPERTY(bool, readerInit);

	/// DataMatrix: only consider square symbol versions
	ZX_RO_PROPERTY(bool, forceSquare);

	/// Specify number of columns (e.g. for DataBarExpStk, PDF417)
	ZX_RO_PROPERTY(int, columns);

	/// Specify number of rows (e.g. for DataBarExpStk, PDF417)
	ZX_RO_PROPERTY(int, rows);

	/// Specify the version/size of the symbol (most 2D symbologies)
	ZX_RO_PROPERTY(int, version);

	/// Specify dataMask to use (QRCode/MicroQRCode)
	ZX_RO_PROPERTY(int, dataMask);

#undef ZX_RO_PROPERTY
};

/**
 * @brief Generate Barcode from unicode text
 *
 * @param contents  UTF-8 string to encode into a barcode
 * @param options  CreatorOptions (including BarcodeFormat)
 */
Barcode CreateBarcodeFromText(std::string_view contents, const CreatorOptions& options);

/**
 * @brief Generate Barcode from raw binary data
 *
 * @param data  array of bytes to encode into a barcode
 * @param size  size of byte array
 * @param options  CreatorOptions (including BarcodeFormat)
 */
Barcode CreateBarcodeFromBytes(const void* data, int size, const CreatorOptions& options);

#if defined(__cpp_lib_char8_t)
Barcode CreateBarcodeFromText(std::u8string_view contents, const CreatorOptions& options);
#endif

#if defined(__cpp_lib_ranges)
template <typename R>
requires std::ranges::contiguous_range<R> && std::ranges::sized_range<R> && (sizeof(std::ranges::range_value_t<R>) == 1)
Barcode CreateBarcodeFromBytes(const R& contents, const CreatorOptions& options)
{
	return CreateBarcodeFromBytes(std::ranges::data(contents), std::ranges::size(contents), options);
}
#else
template <typename R>
Barcode CreateBarcodeFromBytes(const R& contents, const CreatorOptions& options)
{
	return CreateBarcodeFromBytes(std::data(contents), std::size(contents), options);
}
#endif

} // ZXing
