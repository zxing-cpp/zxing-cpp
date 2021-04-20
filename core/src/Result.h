#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BarcodeFormat.h"
#include "ByteArray.h"
#include "DecodeStatus.h"
#include "Quadrilateral.h"
#include "ResultMetadata.h"
#include "ResultPoint.h"
#include "StructuredAppend.h"

#include <string>
#include <utility>
#include <vector>

namespace ZXing {

class DecoderResult;

using Position = QuadrilateralI;

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*
* @author Sean Owen
*/
class Result
{
public:
	explicit Result(DecodeStatus status) : _status(status) {}

	Result(std::wstring&& text, Position&& position, BarcodeFormat format, ByteArray&& rawBytes = {},
		   const bool readerInit = false);

	// 1D convenience constructor
	Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, ByteArray&& rawBytes = {},
		   const bool readerInit = false);

	Result(DecoderResult&& decodeResult, Position&& position, BarcodeFormat format);

	bool isValid() const {
		return StatusIsOK(_status);
	}

	DecodeStatus status() const {
		return _status;
	}

	BarcodeFormat format() const {
		return _format;
	}

	[[deprecated]]
	void setFormat(BarcodeFormat format) {
		_format = format;
	}

	const std::wstring& text() const {
		return _text;
	}

	[[deprecated]]
	void setText(std::wstring&& text) {
		_text = std::move(text);
	}

	const Position& position() const {
		return _position;
	}
	void setPosition(Position pos) {
		_position = pos;
	}

	int orientation() const; //< orientation of barcode in degree, see also Position::orientation()

	const ByteArray& rawBytes() const {
		return _rawBytes;
	}
	
	int numBits() const {
		return _numBits;
	}

	const std::wstring& ecLevel() const {
		return _ecLevel;
	}

	[[deprecated]]
	std::vector<ResultPoint> resultPoints() const {
		return {position().begin(), position().end()};
	}

	[[deprecated]]
	const ResultMetadata& metadata() const {
		return _metadata;
	}

	[[deprecated]]
	ResultMetadata& metadata() {
		return _metadata;
	}

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
	bool isPartOfSequence() const { return sequenceSize() > -1; }

	/**
	 * @brief readerInit Set if Reader Initialisation/Programming symbol.
	 */
	bool readerInit() const {
		return _readerInit;
	}

private:
	DecodeStatus _status = DecodeStatus::NoError;
	BarcodeFormat _format = BarcodeFormat::None;
	std::wstring _text;
	Position _position;
	ByteArray _rawBytes;
	int _numBits = 0;
	std::wstring _ecLevel;
	ResultMetadata _metadata;
	StructuredAppendInfo _sai;
	bool _readerInit = false;
};

} // ZXing
