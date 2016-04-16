#pragma once
/*
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
#include "ZXString.h"

#include <memory>
#include <list>

namespace ZXing {

class CustomData;

/**
* <p>Encapsulates the result of decoding a matrix of bits. This typically
* applies to 2D barcode formats. For now it contains the raw bytes obtained,
* as well as a String interpretation of those bytes, if applicable.</p>
*/
class DecoderResult
{
	bool _valid = false;
	ByteArray _rawBytes;
	String _text;
	std::list<ByteArray> _byteSegments;
	String _ecLevel;
	int _errorsCorrected = -1;
	int _erasures = -1;
	int _structuredAppendSequenceNumber = 0;
	int _structuredAppendParity = 0;
	std::shared_ptr<CustomData> _extra;

public:
	DecoderResult() {}
	DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const String& ecLevel);
	DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const String& ecLevel, int saSequence, int saParity);

	bool isValid() const {
		return _valid;
	}

	const ByteArray& rawBytes() const {
		return _rawBytes;
	}

	const String& text() const {
		return _text;
	}

	const std::list<ByteArray>& byteSegments() const {
		return _byteSegments;
	}

	String ecLevel() const {
		return _ecLevel;
	}

	int errorsCorrected() const {
		return _errorsCorrected;
	}

	int erasures() const {
		return _erasures;
	}

	bool hasStructuredAppend() const {
		return _structuredAppendParity >= 0 && _structuredAppendSequenceNumber >= 0;
	}

	int structuredAppendParity() const {
		return _structuredAppendParity;
	}

	int structuredAppendSequenceNumber() const {
		return _structuredAppendSequenceNumber;
	}

	std::shared_ptr<CustomData> extra() const {
		return _extra;
	}

	void setExtra(const std::shared_ptr<CustomData>& ex) {
		_extra = ex;
	}
};

} // ZXing
