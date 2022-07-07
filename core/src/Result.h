/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "ByteArray.h"
#include "Content.h"
#include "DecodeStatus.h"
#include "Error.h"
#include "Quadrilateral.h"
#include "StructuredAppend.h"

#include <string>
#include <vector>

namespace ZXing {

class DecoderResult;

using Position = QuadrilateralI;

/**
 * @brief The Result class encapsulates the result of decoding a barcode within an image.
 */
class Result
{
	/**
	 * @brief utf8/utf16 is the bytes() content converted to utf8/16 based on ECI or guessed character set information
	 *
	 * Note: these two properties might only be available while transitioning text() from std::wstring to std::string. time will tell.
	 * see https://github.com/nu-book/zxing-cpp/issues/338 for a background discussion on the issue.
	 */
	std::string utf8() const;
	std::wstring utf16() const;

public:
	Result() = default;

	// linear symbology convenience constructor
	Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, SymbologyIdentifier si, Error error = {},
		   ByteArray&& rawBytes = {}, bool readerInit = false, const std::string& ai = {});

	Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format);

	bool isValid() const { return format() != BarcodeFormat::None && !error(); }

	const Error& error() const { return _error; }

	[[deprecated]] DecodeStatus status() const;

	BarcodeFormat format() const { return _format; }

	/**
	 * @brief bytes is the raw / standard content without any modifications like character set conversions
	 */
	const ByteArray& bytes() const;

	/**
	 * @brief bytesECI is the raw / standard content following the ECI protocol
	 */
	ByteArray bytesECI() const;

#ifdef ZX_USE_UTF8
	std::string text() const { return utf8(); }
	std::string ecLevel() const { return _ecLevel; }
#else
#pragma message( \
	"Warning: the return type of text() and ecLevel() will change to std::string. Please #define ZX_USE_UTF8 to transition before the next release.")
	std::wstring text() const { return utf16(); }
	std::wstring ecLevel() const { return {_ecLevel.begin(), _ecLevel.end()}; }
#endif

#if 0 // disabled until final API decission is made
	/**
	 * @brief utf8ECI is the standard content following the ECI protocol with every character set ECI segment transcoded to utf8
	 */
	std::string utf8ECI() const;
#endif

	/**
	 * @brief contentType gives a hint to the type of content found (Text/Binary/GS1/etc.)
	 */
	ContentType contentType() const;

	/**
	 * @brief hasECI specifies wheter or not an ECI tag was found
	 */
	bool hasECI() const;

	const Position& position() const { return _position; }
	void setPosition(Position pos) { _position = pos; }

	/**
	 * @brief orientation of barcode in degree, see also Position::orientation()
	 */
	int orientation() const;

	/**
	 * @brief isMirrored is the symbol mirrored (currently only supported by QRCode and DataMatrix)
	 */
	bool isMirrored() const { return _isMirrored; }

	/// see bytes() above for a proper replacement of rawByes
	[[deprecated]] const ByteArray& rawBytes() const { return _rawBytes; }
	[[deprecated]] int numBits() const { return _numBits; }

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
	bool readerInit() const { return _readerInit; }

	/**
	 * @brief How many lines have been detected with this code (applies only to linear symbologies)
	 */
	int lineCount() const { return _lineCount; }

	// only for internal use
	void incrementLineCount() { ++_lineCount; }
	Result& setCharacterSet(const std::string& defaultCS);

	bool operator==(const Result& o) const;

	friend Result MergeStructuredAppendSequence(const std::vector<Result>& results);

private:
	BarcodeFormat _format = BarcodeFormat::None;
	Content _content;
	Error _error;
	Position _position;
	ByteArray _rawBytes;
	int _numBits = 0;
	std::string _ecLevel;
	StructuredAppendInfo _sai;
	bool _isMirrored = false;
	bool _readerInit = false;
	int _lineCount = 0;
};

using Results = std::vector<Result>;

// Consider this an internal function that can change/disappear anytime without notice
inline Result FirstOrDefault(Results&& results)
{
	return results.empty() ? Result() : std::move(results.front());
}

/**
 * @brief Merge a list of Results from one Structured Append sequence to a single result
 */
Result MergeStructuredAppendSequence(const Results& results);

/**
 * @brief Automatically merge all Structured Append sequences found in the given results
 */
Results MergeStructuredAppendSequences(const Results& results);

} // ZXing
