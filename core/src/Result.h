#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "Quadrilateral.h"
#include "ResultPoint.h"
#include "ResultMetadata.h"
#include "DecodeStatus.h"

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

	Result(std::wstring&& text, Position&& position, BarcodeFormat format, ByteArray&& rawBytes = {});

	// 1D convenience constructor
	Result(const std::string& text, int y, int xStart, int xStop, BarcodeFormat format, ByteArray&& rawBytes = {});

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
	void setFormat(BarcodeFormat format) {
		_format = format;
	}

	const std::wstring& text() const {
		return _text;
	}
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

	[[deprecated]]
	std::vector<ResultPoint> resultPoints() const {
		return {position().begin(), position().end()};
	}

	const ResultMetadata& metadata() const {
		return _metadata;
	}

	ResultMetadata& metadata() {
		return _metadata;
	}

private:
	DecodeStatus _status = DecodeStatus::NoError;
	BarcodeFormat _format = BarcodeFormat::NONE;
	std::wstring _text;
	Position _position;
	ByteArray _rawBytes;
	int _numBits = 0;
	ResultMetadata _metadata;
};

} // ZXing
