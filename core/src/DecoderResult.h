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
#include "DecodeStatus.h"

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
	DecodeStatus _status = DecodeStatus::NoError;
	ByteArray _rawBytes;
	int _numBits = 0;
	std::wstring _text;
	std::list<ByteArray> _byteSegments;
	std::wstring _ecLevel;
	int _errorsCorrected = -1;
	int _erasures = -1;
	int _structuredAppendSequenceNumber = 0;
	int _structuredAppendParity = 0;
	std::shared_ptr<CustomData> _extra;

	DecoderResult(const DecoderResult &) = delete;
	DecoderResult& operator=(const DecoderResult &) = delete;

public:
	DecoderResult(DecodeStatus status) : _status(status) {}
	DecoderResult(ByteArray&& rawBytes, std::wstring&& text) : _rawBytes(std::move(rawBytes)), _text(std::move(text)) {
		_numBits = 8 * rawBytes.length();
	}

	DecoderResult() = default;
	DecoderResult(DecoderResult&&) = default;
	DecoderResult& operator=(DecoderResult&&) = default;

	bool isValid() const { return StatusIsOK(_status); }
	DecodeStatus errorCode() const { return _status; }

	const ByteArray& rawBytes() const & { return _rawBytes; }
	ByteArray&& rawBytes() && { return std::move(_rawBytes); }
	const std::wstring& text() const & { return _text; }
	std::wstring&& text() && { return std::move(_text); }

	// Simple macro to set up getter/setter methods that save lots of boilerplate.
	// It sets up a standard 'const & () const', 2 setters for setting lvalues via
	// copy and 2 for setting rvalues via move. They are provided each to work
	// either on lvalues (normal 'void (...)') or on rvalues (returning '*this' as
	// rvalue). The latter can be used to optionally initialize a temporary in a
	// return statement, e.g.
	//    return DecoderResult(bytes, text).setEcLevel(level);
#define ZX_PROPERTY(TYPE, GETTER, SETTER) \
	const TYPE& GETTER() const & { return _##GETTER; } \
	TYPE&& GETTER() && { return std::move(_##GETTER); } \
	void SETTER(const TYPE& v) & { _##GETTER = v; } \
	void SETTER(TYPE&& v) & { _##GETTER = std::move(v); } \
	DecoderResult&& SETTER(const TYPE& v) && { _##GETTER = v; return std::move(*this); } \
	DecoderResult&& SETTER(TYPE&& v) && { _##GETTER = std::move(v); return std::move(*this); }

	ZX_PROPERTY(int, numBits, setNumBits)
	ZX_PROPERTY(std::list<ByteArray>, byteSegments, setByteSegments)
	ZX_PROPERTY(std::wstring, ecLevel, setEcLevel)
	ZX_PROPERTY(int, errorsCorrected, setErrorsCorrected)
	ZX_PROPERTY(int, erasures, setErasures)
	ZX_PROPERTY(int, structuredAppendParity, setStructuredAppendParity)
	ZX_PROPERTY(int, structuredAppendSequenceNumber, setStructuredAppendSequenceNumber)
	ZX_PROPERTY(std::shared_ptr<CustomData>, extra, setExtra)

#undef ZX_PROPERTY

	bool hasStructuredAppend() const { return _structuredAppendParity >= 0 && _structuredAppendSequenceNumber >= 0; }
};

} // ZXing
