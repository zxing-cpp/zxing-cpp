/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Version.h"

#include <stdint.h>

// FLAGS[0]: l == linear, s == stacked, m == matrix
// FLAGS[1]: r == readable
// FLAGS[2]: w == writable (MultiformatWriter))
// FLAGS[3]: g == GS1

// clang-format off
//    NAME,            SYM, VAR, FLAGS, ZINT, ENABLED,                 HRI
#define ZX_BCF_LIST(X) \
	X(None,              0,   0,  "    ",   0, 1,                       "None") \
	X(All,              '*', '*', "    ",   0, 1,                       "All") \
	X(AllReadable,      '*', 'r', "    ",   0, 1,                       "All Readable") \
	X(AllCreatable,     '*', 'w', "    ",   0, 1,                       "All Creatable") \
	X(AllLinear,        '*', 'l', "    ",   0, 1,                       "All Linear") \
	X(AllStacked,       '*', 's', "    ",   0, 1,                       "All Stacked") \
	X(AllMatrix,        '*', 'm', "    ",   0, 1,					    "All Matrix") \
	X(AllGS1,           '*', 'g', "    ",   0, 1,                       "All GS1") \
	X(Codabar,          'F', ' ', "lrw ",  18, ZXING_ENABLE_1D,         "Codabar") \
	X(Code39,           'A', ' ', "lrw ",   8, ZXING_ENABLE_1D,         "Code 39") \
	X(PZN,              'A', 'p', "lr  ",   8, ZXING_ENABLE_1D,         "Pharmazentralnummer") \
	X(Code93,           'G', ' ', "lrw ",  25, ZXING_ENABLE_1D,         "Code 93") \
	X(Code128,          'C', ' ', "lrwg",  20, ZXING_ENABLE_1D,         "Code 128") \
	X(ITF,              'I', ' ', "lrw ",   3, ZXING_ENABLE_1D,         "ITF") \
	X(DataBar,          'e', ' ', "lr  ",  29, ZXING_ENABLE_1D,         "DataBar") \
	X(DataBarOmD,       'e', 'o', "lr  ",  29, ZXING_ENABLE_1D,         "DataBar Omnidirectional") \
	X(DataBarLtd,       'e', 'l', "lr  ",  30, ZXING_ENABLE_1D,         "DataBar Limited") \
	X(DataBarExp,       'e', 'e', "lr g",  31, ZXING_ENABLE_1D,         "DataBar Expanded") \
	X(EANUPC,           'E', ' ', "lr  ",  15, ZXING_ENABLE_1D,         "EAN/UPC") \
	X(EAN13,            'E', '1', "lrw ",  15, ZXING_ENABLE_1D,         "EAN-13") \
	X(EAN8,             'E', '8', "lrw ",  10, ZXING_ENABLE_1D,         "EAN-8") \
	X(EAN5,             'E', '5', "l   ",  12, ZXING_ENABLE_1D,         "EAN-5") \
	X(EAN2,             'E', '2', "l   ",  11, ZXING_ENABLE_1D,         "EAN-2") \
	X(ISBN,             'E', 'i', "lr  ",  69, ZXING_ENABLE_1D,         "ISBN") \
	X(UPCA,             'E', 'a', "lrw ",  34, ZXING_ENABLE_1D,         "UPC-A") \
	X(UPCE,             'E', 'e', "lrw ",  37, ZXING_ENABLE_1D,         "UPC-E") \
	X(DXFilmEdge,       'X', 'x', "lr  ", 147, ZXING_ENABLE_1D,         "DX Film Edge") \
	X(PDF417,           'L', ' ', "srw ",  55, ZXING_ENABLE_PDF417,     "PDF417") \
	X(CompactPDF417,    'L', 'c', "sr  ",  56, ZXING_ENABLE_PDF417,     "Compact PDF417") \
	X(MicroPDF417,      'L', 'm', "s   ",  84, ZXING_ENABLE_PDF417,     "MicroPDF417") \
	X(Aztec,            'z', ' ', "mrwg",  92, ZXING_ENABLE_AZTEC,      "Aztec") \
	X(AztecCode,        'z', 'c', "mrwg",  92, ZXING_ENABLE_AZTEC,      "Aztec Code") \
	X(AztecRune,        'z', 'r', "mr  ", 128, ZXING_ENABLE_AZTEC,      "Aztec Rune") \
	X(QRCode,           'Q', ' ', "mrwg",  58, ZXING_ENABLE_QRCODE,     "QR Code") \
	X(QRCodeModel1,     'Q', '1', "mr  ",   0, ZXING_ENABLE_QRCODE,     "QR Code Model 1") \
	X(MicroQRCode,      'Q', 'm', "mr  ",  97, ZXING_ENABLE_QRCODE,     "Micro QR Code") \
	X(RMQRCode,         'Q', 'r', "mr g", 145, ZXING_ENABLE_QRCODE,     "rMQR Code") \
	X(DataMatrix,       'd', ' ', "mrwg",  71, ZXING_ENABLE_DATAMATRIX, "Data Matrix") \
	X(MaxiCode,         'U', ' ', "mr  ",  57, ZXING_ENABLE_MAXICODE,   "MaxiCode") \
	/* Add new formats here */
// clang-format on

#define ZX_BCF_ID(SYM, VAR) (((uint8_t)(SYM) << 0) | ((uint8_t)(VAR) << 8))

#ifdef __cplusplus

#include <array>
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ZXing {

/**
* Enumerates barcode formats known to this package.
*/
enum class BarcodeFormat : unsigned int
{
#define X(NAME, SYM, VAR, FLAGS, ZINT, ENABLED, HRI) NAME = ZX_BCF_ID(SYM, VAR),
	ZX_BCF_LIST(X)
#undef X
	DataBarExpanded [[deprecated("Use DataBarExp instead")]] = DataBarExp,
	DataBarLimited [[deprecated("Use DataBarLtd instead")]] = DataBarLtd,
	LinearCodes [[deprecated("Use AllLinear instead")]] = AllLinear,
	MatrixCodes [[deprecated("Use AllMatrix instead")]] = AllMatrix,
	Any [[deprecated("Use All instead")]] = All,
};

inline char SymbologyKey(const BarcodeFormat& format)
{
	return uint32_t(format) & 0xFF;
}

inline char VariantKey(const BarcodeFormat& format)
{
	return (uint32_t(format) >> 8) & 0xFF;
}

inline std::string IdStr(const BarcodeFormat& format)
{
	return {']', SymbologyKey(format), VariantKey(format)};
}

/// @brief Returns the symbology (base type) of the given barcode format (e.g. EAN/UPC for EAN13, EAN8, UPCA, etc.).
BarcodeFormat Symbology(BarcodeFormat format);

/// @brief Returns the human-readable name of the given barcode format.
std::string_view Name(BarcodeFormat format);

/// Test if the two BarcodeFormats have a non-empty intersection (e.g. AllMatrix & QRCode)
bool operator&(BarcodeFormat a, BarcodeFormat b);

template <std::size_t N>
using BarcodeFormatArray = std::array<BarcodeFormat, N>;

constexpr BarcodeFormatArray<2> operator|(BarcodeFormat a, BarcodeFormat b)
{
	return {a, b};
}

template <std::size_t N>
constexpr BarcodeFormatArray<N + 1> operator|(BarcodeFormatArray<N> bts, BarcodeFormat bt)
{
	BarcodeFormatArray<N + 1> ret{};
	for (std::size_t i = 0; i < N; ++i)
		ret[i] = bts[i];
	ret[N] = bt;
	return ret;
}

template <std::size_t N>
constexpr bool operator&(BarcodeFormat lhs, const BarcodeFormatArray<N>& rhs)
{
	return std::any_of(rhs.begin(), rhs.end(), [lhs](BarcodeFormat bt) { return lhs & bt; });
}

/**
 * @brief Parse a string into a BarcodeFormat. '-', '_', '/' and ' ' are optional.
 * @throws std::invalid_parameter if the string can not be fully parsed.
 */
BarcodeFormat BarcodeFormatFromString(std::string_view str);

std::string ToString(BarcodeFormat format);

/**
 * @class BarcodeFormats
 * @brief A small container representing a collection of BarcodeFormat values.
 *
 * BarcodeFormats encapsulates an ordered collection of BarcodeFormat values and
 * provides convenient construction, iteration, and set-like operations.
 *
 * General behavior and invariants:
 * - The internal representation is kept in a canonical form (sorted, no duplicates)
 *   using normalize().
 * - The API exposes read-only access and iteration.
 */
class BarcodeFormats
{
	std::vector<BarcodeFormat> formats_;

	void normalize();

public:
	BarcodeFormats() = default;
	BarcodeFormats(BarcodeFormat f) : formats_{f} {}

	/// @brief Constructs a collection from a compile-time array of BarcodeFormat values.
	/// @example BarcodeFormats formats = BarcodeFormat::QRCode | BarcodeFormat::EAN13;
	template <std::size_t N>
	BarcodeFormats(BarcodeFormatArray<N> formats) : formats_{formats.begin(), formats.end()}
	{
		normalize();
	}

	BarcodeFormats(std::vector<BarcodeFormat>&& formats) : formats_(std::move(formats)) { normalize(); }

	/// @brief Constructs a collection from a textual representation (e.g. a comma-separated list of format identifiers).
	explicit BarcodeFormats(std::string_view str);

	BarcodeFormats(const BarcodeFormats&) = default;
	BarcodeFormats(BarcodeFormats&&) = default;
	BarcodeFormats& operator=(const BarcodeFormats&) = default;
	BarcodeFormats& operator=(BarcodeFormats&&) = default;
	bool operator==(const BarcodeFormats& o) const noexcept { return formats_ == o.formats_; }
	bool operator!=(const BarcodeFormats& o) const noexcept { return !(*this == o); }
	// ~BarcodeFormats() = default;

	auto begin() const noexcept { return formats_.cbegin(); }
	auto end() const noexcept { return formats_.cend(); }

	using value_type = typename std::vector<BarcodeFormat>::value_type;
	using pointer = typename std::vector<BarcodeFormat>::pointer;
	auto data() const noexcept { return formats_.data(); }

	bool empty() const noexcept { return formats_.empty(); }
	int size() const noexcept { return static_cast<int>(formats_.size()); }

	// BarcodeFormats&& operator|(BarcodeFormat bt) &&;
	// BarcodeFormats&& operator|(const BarcodeFormats& other) &&;

	/// @brief Returns a BarcodeFormats containing the intersection of this collection and other.
	BarcodeFormats operator&(const BarcodeFormats& other);

	/// @brief Returns a list of available/supported barcode formats, optionally filtered by the provided formats.
	/// @example BarcodeFormats::list(BarcodeFormat::AllReadable);
	static BarcodeFormats list(const BarcodeFormats& filter = {});

	[[deprecated]] inline int count() const noexcept { return size(); }
	[[deprecated]] inline bool testFlag(BarcodeFormat format) const noexcept
	{
		return std::any_of(begin(), end(), [format](BarcodeFormat f) { return f & format; });
	}
	[[deprecated]] inline bool testFlags(const BarcodeFormats& formats) const noexcept
	{
		return std::any_of(begin(), end(), [formats](BarcodeFormat fo) {
			return std::any_of(formats.begin(), formats.end(), [fo](BarcodeFormat fi) { return fo & fi; });
		});
	}
};

inline bool operator&(BarcodeFormat lhs, const BarcodeFormats& rhs)
{
	return std::any_of(rhs.begin(), rhs.end(), [lhs](BarcodeFormat bf) { return lhs & bf; });
}

/**
 * @brief Parse a string into a set of BarcodeFormats.
 * Separators can be (any combination of) '|' or ','.
 * Input can be lower case and any of '-', '_', '/' or ' ' are optional.
 * e.g. "EAN-8 | qrcode, Itf" would be parsed into [EAN8, QRCode, ITF].
 * @throws std::invalid_parameter if the string can not be fully parsed.
 */
BarcodeFormats BarcodeFormatsFromString(std::string_view str);

std::string ToString(const BarcodeFormats& formats);

} // namespace ZXing

#endif // __cplusplus

#if 0
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Flags.h"

#include <string>
#include <string_view>

namespace ZXing {

/**
* Enumerates barcode formats known to this package.
*/
enum class BarcodeFormat
{
	// The values are an implementation detail. The c++ use-case (ZXing::Flags) could have been designed such that it
	// would not have been necessary to explicitly set the values to single bit constants. This has been done to ease
	// the interoperability with C-like interfaces, the python and the Qt wrapper.
	None            = 0,         ///< Used as a return value if no valid barcode has been detected
	Aztec           = (1 << 0),  ///< Aztec
	Codabar         = (1 << 1),  ///< Codabar
	Code39          = (1 << 2),  ///< Code39
	Code93          = (1 << 3),  ///< Code93
	Code128         = (1 << 4),  ///< Code128
	DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
	DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
	DataMatrix      = (1 << 7),  ///< DataMatrix
	EAN8            = (1 << 8),  ///< EAN-8
	EAN13           = (1 << 9),  ///< EAN-13
	ITF             = (1 << 10), ///< ITF (Interleaved Two of Five)
	MaxiCode        = (1 << 11), ///< MaxiCode
	PDF417          = (1 << 12), ///< PDF417
	QRCode          = (1 << 13), ///< QR Code
	UPCA            = (1 << 14), ///< UPC-A
	UPCE            = (1 << 15), ///< UPC-E
	MicroQRCode     = (1 << 16), ///< Micro QR Code
	RMQRCode        = (1 << 17), ///< Rectangular Micro QR Code
	DXFilmEdge      = (1 << 18), ///< DX Film Edge Barcode
	DataBarLimited  = (1 << 19), ///< GS1 DataBar Limited

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | DataBarLimited
				  | DXFilmEdge | UPCA | UPCE,
	MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode | RMQRCode,
	Any         = LinearCodes | MatrixCodes,

	_max = DataBarLimited, ///> implementation detail, don't use
};

ZX_DECLARE_FLAGS(BarcodeFormats, BarcodeFormat)

std::string ToString(BarcodeFormat format);
std::string ToString(BarcodeFormats formats);

/**
 * @brief Parse a string into a BarcodeFormat. '-' and '_' are optional.
 * @return None if str can not be parsed as a valid enum value
 */
BarcodeFormat BarcodeFormatFromString(std::string_view str);

/**
 * @brief Parse a string into a set of BarcodeFormats.
 * Separators can be (any combination of) '|', ',' or ' '.
 * Underscores are optional and input can be lower case.
 * e.g. "EAN-8 qrcode, Itf" would be parsed into [EAN8, QRCode, ITF].
 * @throws std::invalid_parameter if the string can not be fully parsed.
 */
BarcodeFormats BarcodeFormatsFromString(std::string_view str);

} // ZXing
#endif // 0
