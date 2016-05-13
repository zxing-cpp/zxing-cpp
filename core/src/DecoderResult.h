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

#include <memory>
#include <list>
#include <string>

namespace ZXing {

class CustomData;

/**
* <p>Encapsulates the result of decoding a matrix of bits. This typically
* applies to 2D barcode formats. For now it contains the raw bytes obtained,
* as well as a string interpretation of those bytes, if applicable.</p>
*
* @author Sean Owen
*/
class DecoderResult
{
	ByteArray _rawBytes;
	std::wstring _text;
	std::list<ByteArray> _byteSegments;
	std::wstring _ecLevel;
	int _errorsCorrected = -1;
	int _erasures = -1;
	int _structuredAppendSequenceNumber = 0;
	int _structuredAppendParity = 0;
	std::shared_ptr<CustomData> _extra;

public:
	//explicit DecoderResult(ErrorStatus status);
	//DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const std::string& ecLevel, int saSequence, int saParity);
	//DecoderResult(const ByteArray& rawBytes, const String& text, std::list<ByteArray>& byteSegments, const std::string& ecLevel);
	DecoderResult() {}
	DecoderResult(const DecoderResult &) = delete;
	DecoderResult& operator=(const DecoderResult &) = delete;

	const ByteArray& rawBytes() const { return _rawBytes; }
	void setRawBytes(const ByteArray& bytes) { _rawBytes = bytes; }

	const std::wstring& text() const { return _text; }
	void setText(const std::wstring& txt) { _text = txt; }

	const std::list<ByteArray>& byteSegments() const { return _byteSegments; }
	void setByteSegments(const std::list<ByteArray>& segments) { _byteSegments = segments; }

	std::wstring ecLevel() const { return _ecLevel; }
	void setEcLevel(const std::wstring& level) { _ecLevel = level; }

	int errorsCorrected() const { return _errorsCorrected; }
	void setErrorsCorrected(int ec) { _errorsCorrected = ec; }

	int erasures() const { return _erasures; }
	void setErasures(int e) { _erasures = e; }

	bool hasStructuredAppend() const { return _structuredAppendParity >= 0 && _structuredAppendSequenceNumber >= 0; }

	int structuredAppendParity() const { return _structuredAppendParity; }
	void setStructuredAppendParity(int p) { _structuredAppendParity = p; }

	int structuredAppendSequenceNumber() const { return _structuredAppendSequenceNumber; }
	void setStructuredAppendSequenceNumber(int s) { _structuredAppendSequenceNumber = s; }

	std::shared_ptr<CustomData> extra() const { return _extra; }
	void setExtra(const std::shared_ptr<CustomData> e) { _extra = e; }
};

} // ZXing
