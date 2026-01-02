/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ByteArray.h"
#include "ContentType.h"
#include "ReaderOptions.h"
#include "Error.h"
#include "ImageView.h"
#include "Quadrilateral.h"
#include "StructuredAppend.h"

#include <memory>
#include <string>
#include <vector>

#ifdef ZXING_USE_ZINT
extern "C" struct zint_symbol;
struct zint_symbol_deleter
{
	void operator()(zint_symbol* p) const noexcept;
};
using unique_zint_symbol = std::unique_ptr<zint_symbol, zint_symbol_deleter>;
#endif

namespace ZXing {

struct SymbologyIdentifier;
class DecoderResult;
class DetectorResult;
class CreatorOptions;
class WriterOptions;
class Barcode;

using Position = QuadrilateralI;
using Barcodes = std::vector<Barcode>;

class BitMatrix;
class BinaryBitmap;
namespace OneD {
class RowReader;
Barcodes DoDecode(const std::vector<std::unique_ptr<RowReader>>&, const BinaryBitmap&, bool, bool, bool, int, int, bool);
}

namespace BarcodeExtra {
	#define ZX_EXTRA(NAME) static constexpr auto NAME = #NAME
	ZX_EXTRA(DataMask); // QRCodes
	ZX_EXTRA(Version);
	ZX_EXTRA(EanAddOn); // EAN/UPC
	ZX_EXTRA(UPCE);
	ZX_EXTRA(ReaderInit);
	#undef ZX_EXTRA
} // namespace BarcodeExtra

/**
 * @brief The Barcode class encapsulates a decoded or created barcode symbol.
 */
class Barcode
{
	struct Data;

	std::shared_ptr<Data> d;

	void setIsInverted(bool v);
	void setPosition(Position pos);
	void incrementLineCount();
	const BitMatrix& symbolMatrix() const;
	Barcode& setReaderOptions(const ReaderOptions& opts);
#ifdef ZXING_USE_ZINT
	void zint(unique_zint_symbol&& z);
#endif

	friend Barcode MergeStructuredAppendSequence(const Barcodes&);
	friend Barcodes ReadBarcodes(const ImageView&, const ReaderOptions&);
	friend Barcode CreateBarcode(const void*, int, int, const CreatorOptions&);
	friend Image WriteBarcodeToImage(const Barcode&, const WriterOptions&);
	friend Barcodes OneD::DoDecode(const std::vector<std::unique_ptr<OneD::RowReader>>&, const BinaryBitmap&, bool, bool, bool, int,
								   int, bool);

public:
	Barcode();

	// linear symbology convenience constructor
	Barcode(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, SymbologyIdentifier si, Error error = {},
			std::string extra = {});

	Barcode(DecoderResult&& decodeResult, DetectorResult&& detectorResult, BarcodeFormat format);

	bool isValid() const;

	const Error& error() const;

	BarcodeFormat format() const;

	/**
	 * @brief bytes is the raw / standard content without any modifications like character set conversions
	 */
	const ByteArray& bytes() const; // TODO 3.0: replace ByteArray with std::vector<uint8_t>

	/**
	 * @brief bytesECI is the raw / standard content following the ECI protocol
	 */
	ByteArray bytesECI() const;

	/**
	 * @brief text returns the bytes() content rendered to unicode/utf8 text accoring to specified TextMode
	 */
	std::string text(TextMode mode) const;

	/**
	 * @brief text returns the bytes() content rendered to unicode/utf8 text accoring to the TextMode set in the ReaderOptions
	 */
	std::string text() const;

	/**
	 * @brief ecLevel returns the error correction level of the symbol (empty string if not applicable)
	 */
	std::string ecLevel() const;

	/**
	 * @brief contentType gives a hint to the type of content found (Text/Binary/GS1/etc.)
	 */
	ContentType contentType() const;

	/**
	 * @brief hasECI specifies wheter or not an ECI tag was found
	 */
	bool hasECI() const;

	const Position& position() const;

	/**
	 * @brief orientation of barcode in degree, see also Position::orientation()
	 */
	int orientation() const;

	/**
	 * @brief isMirrored is the symbol mirrored (currently only supported by QRCode and DataMatrix)
	 */
	bool isMirrored() const;
	/**
	 * @brief isInverted is the symbol inverted / has reveresed reflectance (see ReaderOptions::tryInvert)
	 */
	bool isInverted() const;

	/**
	 * @brief symbologyIdentifier Symbology identifier "]cm" where "c" is symbology code character, "m" the modifier.
	 */
	std::string symbologyIdentifier() const;

	/**
	 * @brief sequenceSize number of symbols in a structured append sequence.
	 *
	 * If this is not part of a structured append sequence, the returned value is -1.
	 * If it is a structured append symbol but the total number of symbols is unknown, the
	 * returned value is 0 (see PDF417 if optional "Segment Count" not given).
	 */
	int sequenceSize() const;

	/**
	 * @brief sequenceIndex the 0-based index of this symbol in a structured append sequence.
	 */
	int sequenceIndex() const;

	/**
	 * @brief sequenceId id to check if a set of symbols belongs to the same structured append sequence.
	 *
	 * If the symbology does not support this feature, the returned value is empty (see MaxiCode).
	 * For QR Code, this is the parity integer converted to a string.
	 * For PDF417 and DataMatrix, this is the "fileId".
	 */
	std::string sequenceId() const;

	bool isLastInSequence() const { return sequenceSize() == sequenceIndex() + 1; }
	bool isPartOfSequence() const { return sequenceSize() > -1 && sequenceIndex() > -1; }

	/**
	 * @brief readerInit Set if Reader Initialisation/Programming symbol.
	 */
	// [[deprecated]]
	bool readerInit() const { return !extra(BarcodeExtra::ReaderInit).empty(); }

	/**
	 * @brief lineCount How many lines have been detected with this code (applies only to linear symbologies)
	 */
	int lineCount() const;

	/**
	 * @brief version QRCode / DataMatrix / Aztec version or size.
	 */
	std::string version() const;

	ImageView symbol() const;
#ifdef ZXING_USE_ZINT
	zint_symbol* zint() const;
#endif
	std::string extra(std::string_view key = "") const;

	bool operator==(const Barcode& o) const;
};

/**
 * @brief Merge a list of Barcodes from one Structured Append sequence to a single barcode
 */
Barcode MergeStructuredAppendSequence(const Barcodes& results);

/**
 * @brief Automatically merge all Structured Append sequences found in the given list of barcodes
 */
Barcodes MergeStructuredAppendSequences(const Barcodes& barcodes);

} // ZXing
