/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ContentType.h"
#include "Error.h"
#include "ImageView.h"
#include "Quadrilateral.h"
#include "ReaderOptions.h" // for TextMode
#include "Version.h" // ZXING_... macros

#include <memory>
#include <string>
#include <vector>

#ifdef ZXING_USE_ZINT
extern "C" struct zint_symbol;
#endif

namespace ZXing {

class CreatorOptions;
class ReaderOptions;
class WriterOptions;
class Barcode;
struct BarcodeData;

using Position = QuadrilateralI;
using Barcodes = std::vector<Barcode>;

/**
 * @brief String keys for the extra() metadata of Barcode. The values depend on the symbology and may not be present for all barcodes.
 */
namespace BarcodeExtra {
	#define ZX_EXTRA(NAME) static constexpr auto NAME = #NAME
	ZX_EXTRA(DataMask); ///< QRCodes
	ZX_EXTRA(Version);  ///< QRCodes, DataMatrix, MaxiCode (e.g. "12x12" for DataMatrix)
	ZX_EXTRA(EanAddOn); ///< The EAN/UPC add-on content if detected
	ZX_EXTRA(ECLevel);  ///< Error correction level (e.g. "L", "M", "Q", "H" for QRCode)
	ZX_EXTRA(UPCE);     ///< The original (non-normalized) UPC-E code if UPC-E is detected
	ZX_EXTRA(ReaderInit);
	#undef ZX_EXTRA
} // namespace BarcodeExtra

/**
 * @brief The Barcode class encapsulates a decoded or created barcode symbol.
 *
 * Barcode represents a decoded or created barcode symbol, providing access to its content, format,
 * position, and other metadata. It serves as the primary interface for working with barcodes in the library.
 *
 * Barcode objects are obtained from the ReadBarcodes function when scanning an image, or from the
 * CreateBarcodeFrom... functions when generating a new barcode. To convert a Barcode to an image or SVG,
 * use the WriteBarcodeTo... functions.
 *
 * @see ReadBarcodes, CreateBarcodeFromText, CreateBarcodeFromBytes, WriteBarcodeToImage, WriteBarcodeToSVG
 */
class Barcode
{
	using Data = BarcodeData;

	std::shared_ptr<Data> d;

	Barcode& setReaderOptions(const ReaderOptions& opts);

	friend Barcode MergeStructuredAppendSequence(const Barcodes&);
	friend Barcodes ReadBarcodes(const ImageView&, const ReaderOptions&);
	friend Barcode CreateBarcode(const void*, int, int, const CreatorOptions&);
	friend Image WriteBarcodeToImage(const Barcode&, const WriterOptions&);
	friend std::string WriteBarcodeToSVG(const Barcode&, const WriterOptions&);

public:
	Barcode();
	Barcode(Barcode::Data&& data);

	/// Returns whether the barcode is valid, i.e. it contains a successfully decoded or created symbol.
	bool isValid() const;

	/// Returns the error associated with the barcode, if any.
	const Error& error() const;

	/// @brief Returns the BarcodeFormat of the barcode.
	BarcodeFormat format() const;

	/// Returns the symbology of the barcode format (e.g. EAN/UPC for EAN13, EAN8, UPCA, etc.).
	BarcodeFormat symbology() const { return Symbology(format()); }

	/// Returns the raw / standard content without any modifications like character set conversions.
	const std::vector<uint8_t>& bytes() const;

	/// Returns the raw / standard content following the ECI protocol.
	std::vector<uint8_t> bytesECI() const;

	/// Returns the bytes() content rendered to unicode/utf8 text according to specified TextMode.
	std::string text(TextMode mode) const;

	/// Returns the bytes() content rendered to unicode/utf8 text according to the TextMode set in the ReaderOptions.
	std::string text() const;

	/// Returns the content type, giving a hint to the type of content found (Text/Binary/GS1/etc.).
	ContentType contentType() const;

	/// Returns whether or not an ECI tag was found.
	bool hasECI() const;

	/// Returns the position of the barcode in the image as a quadrilateral of 4 points (topLeft, topRight, bottomRight,
	/// bottomLeft).
	const Position& position() const;

	/// Returns the orientation of the barcode in degrees, see also Position::orientation()
	int orientation() const;

	/// Returns whether the symbol is mirrored (currently only supported by QRCode and DataMatrix).
	bool isMirrored() const;
	/// Returns whether the symbol is inverted / has reversed reflectance (see ReaderOptions::tryInvert).
	bool isInverted() const;

	/// Returns the symbology identifier "]cm" where "c" is symbology code character, "m" the modifier.
	std::string symbologyIdentifier() const;

	/// @brief Returns the number of symbols in a structured append sequence.
	///
	/// If this is not part of a structured append sequence, the returned value is -1.
	/// If it is a structured append symbol but the total number of symbols is unknown, the
	/// returned value is 0 (see PDF417 if optional "Segment Count" not given).
	int sequenceSize() const;

	/// Returns the 0-based index of this symbol in a structured append sequence.
	int sequenceIndex() const;

	/// @brief Returns the sequenceId to check if a set of symbols belongs to the same structured append sequence.
	///
	/// If the symbology does not support this feature, the returned value is empty (see MaxiCode).
	/// For QR Code, this is the parity integer converted to a string.
	/// For PDF417 and DataMatrix, this is the "fileId".
	std::string sequenceId() const;

	/// Returns whether this symbol is the last in a structured append sequence.
	bool isLastInSequence() const { return sequenceSize() == sequenceIndex() + 1; }

	/// Returns whether this symbol is part of a structured append sequence.
	bool isPartOfSequence() const { return sequenceSize() > -1 && sequenceIndex() > -1; }

	/// Returns how many lines have been detected with this code (applies only to linear symbologies).
	int lineCount() const;

	/// Returns the bit matrix of the symbol as an ImageView.
	ImageView symbol() const;

	/// @brief Retrieve supplementary metadata associated with this barcode.
	///
	/// Returns a string containing additional and symbology specific information. In form of a JSON object
	/// serialization. The optional parameter @p key can be used to retrieve a specific item only.
	/// Key values are case insensitive. See BarcodeExtra namespace for valid keys.
	/// If the key is not found or there is no info available, an empty string is returned.
	std::string extra(std::string_view key = "") const;

	/// @cond DEPRECATED
	/// Returns the error correction level of the symbol (empty string if not applicable).
	// [[deprecated ("use extra(BarcodeExtra::ECLevel) instead")]]
	std::string ecLevel() const { return extra(BarcodeExtra::ECLevel); }

	/// Returns whether the symbol is a Reader Initialisation/Programming symbol.
	// [[deprecated ("use extra(BarcodeExtra::ReaderInit) instead")]]
	bool readerInit() const { return !extra(BarcodeExtra::ReaderInit).empty(); }

	/// Returns the version QRCode / DataMatrix / Aztec version or size.
	// [[deprecated ("use extra(BarcodeExtra::Version) instead")]]
	std::string version() const { return extra(BarcodeExtra::Version); }
	/// @endcond

#if defined(ZXING_USE_ZINT) && defined(ZXING_EXPERIMENTAL_API)
	zint_symbol* zint() const;
#endif

	bool operator==(const Barcode& o) const;
};

/// Merge a list of Barcodes from one Structured Append sequence to a single barcode.
Barcode MergeStructuredAppendSequence(const Barcodes& barcodes);

/// Automatically merge all Structured Append sequences found in the given list of barcodes.
Barcodes MergeStructuredAppendSequences(const Barcodes& barcodes);

} // ZXing
