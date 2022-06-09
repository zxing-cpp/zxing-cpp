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
public:
	explicit Result(DecodeStatus status) : _status(status) {}

	// 1D convenience constructor
	Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format,
		   SymbologyIdentifier si, ByteArray&& rawBytes = {}, const bool readerInit = false);

	Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format);

	bool isValid() const { return StatusIsOK(_status); }

	DecodeStatus status() const { return _status; }

	BarcodeFormat format() const { return _format; }

	const std::wstring& text() const { return _text; }

	// WARNING: this is an experimental API and may change/disappear
	const ByteArray& bytes() const { return _content.bytes; }
	const ByteArray bytesECI() const { return _content.bytesECI(); }
	const std::string utf8Protocol() const { return _content.utf8Protocol(); }
	const std::string& applicationIndicator() const { return _content.applicationIndicator; }
	ContentType contentType() const { return _content.type(); }
	bool hasECI() const { return _content.hasECI; }
	// END WARNING

	const Position& position() const { return _position; }
	void setPosition(Position pos) { _position = pos; }

	int orientation() const; //< orientation of barcode in degree, see also Position::orientation()

	/**
	 * @brief isMirrored is the symbol mirrored (currently only supported by QRCode and DataMatrix)
	 */
	bool isMirrored() const { return _isMirrored; }

	/// see bytes() above for a proper replacement of rawByes
	[[deprecated]] const ByteArray& rawBytes() const { return _rawBytes; }
	[[deprecated]] int numBits() const { return _numBits; }

	const std::wstring& ecLevel() const { return _ecLevel; }

	/**
	 * @brief symbologyIdentifier Symbology identifier "]cm" where "c" is symbology code character, "m" the modifier.
	 */
	const std::string& symbologyIdentifier() const { return _symbologyIdentifier; }

	/**
	 * @brief sequenceSize number of symbols in a structured append sequence.
	 *
	 * If this is not part of a structured append sequence, the returned value is -1.
	 * If it is a structured append symbol but the total number of symbols is unknown, the
	 * returned value is 0 (see PDF417 if optional "Segment Count" not given).
	 */
	int sequenceSize() const { return _sai.count; }

	/**
	 * @brief sequenceIndex the 0-based index of this symbol in a structured append sequence.
	 */
	int sequenceIndex() const { return _sai.index; }

	/**
	 * @brief sequenceId id to check if a set of symbols belongs to the same structured append sequence.
	 *
	 * If the symbology does not support this feature, the returned value is empty (see MaxiCode).
	 * For QR Code, this is the parity integer converted to a string.
	 * For PDF417 and DataMatrix, this is the "fileId".
	 */
	const std::string& sequenceId() const { return _sai.id; }

	bool isLastInSequence() const { return sequenceSize() == sequenceIndex() + 1; }
	bool isPartOfSequence() const { return sequenceSize() > -1 && sequenceIndex() > -1; }

	/**
	 * @brief readerInit Set if Reader Initialisation/Programming symbol.
	 */
	bool readerInit() const { return _readerInit; }

	/**
	 * @brief How many lines have been detected with this code (applies only to 1D symbologies)
	 */
	int lineCount() const { return _lineCount; }
	void incrementLineCount() { ++_lineCount; }

	bool operator==(const Result& o) const;

	friend Result MergeStructuredAppendSequence(const std::vector<Result>& results);

private:
	DecodeStatus _status = DecodeStatus::NoError;
	BarcodeFormat _format = BarcodeFormat::None;
	Content _content;
	std::wstring _text;
	Position _position;
	ByteArray _rawBytes;
	int _numBits = 0;
	std::wstring _ecLevel;
	std::string _symbologyIdentifier;
	StructuredAppendInfo _sai;
	bool _isMirrored = false;
	bool _readerInit = false;
	int _lineCount = 0;
};

using Results = std::vector<Result>;

/**
 * @brief Merge a list of Results from one Structured Append sequence to a single result
 */
Result MergeStructuredAppendSequence(const Results& results);

/**
 * @brief Automatically merge all structured append sequences found in the given results
 */
Results MergeStructuredAppendSequences(const Results& results);

} // ZXing
