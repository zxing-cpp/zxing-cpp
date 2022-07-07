/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"
#include "Content.h"
#include "Error.h"
#include "StructuredAppend.h"
#include "ZXAlgorithms.h"

#include <memory>
#include <string>
#include <utility>

namespace ZXing {

class CustomData;

class DecoderResult
{
	ByteArray _rawBytes;
	Content _content;
	int _numBits = 0;
	std::string _ecLevel;
	int _lineCount = 0;
	StructuredAppendInfo _structuredAppend;
	bool _isMirrored = false;
	bool _readerInit = false;
	Error _error;
	std::shared_ptr<CustomData> _extra;

	DecoderResult(const DecoderResult &) = delete;
	DecoderResult& operator=(const DecoderResult &) = delete;

public:
	DecoderResult() = default;
	DecoderResult(Error error) : _error(std::move(error)) {}
	DecoderResult(ByteArray&& rawBytes, Content&& bytes) : _rawBytes(std::move(rawBytes)), _content(std::move(bytes))
	{
		_numBits = 8 * Size(_rawBytes);
	}

	DecoderResult(DecoderResult&&) noexcept = default;
	DecoderResult& operator=(DecoderResult&&) = default;

	bool isValid(bool includeErrors = false) const
	{
		return _content.symbology.code != 0 && (!_error || includeErrors);
	}

	const ByteArray& rawBytes() const & { return _rawBytes; }
	ByteArray&& rawBytes() && { return std::move(_rawBytes); }
	const Content& content() const & { return _content; }
	Content&& content() && { return std::move(_content); }

	// to keep the unit tests happy for now:
	std::wstring text() const { return _content.utf16(); }
	std::string symbologyIdentifier() const { return _content.symbology.toString(false); }

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
	ZX_PROPERTY(std::string, ecLevel, setEcLevel)
	ZX_PROPERTY(int, lineCount, setLineCount)
	ZX_PROPERTY(StructuredAppendInfo, structuredAppend, setStructuredAppend)
	ZX_PROPERTY(Error, error, setError)
	ZX_PROPERTY(bool, isMirrored, setIsMirrored)
	ZX_PROPERTY(bool, readerInit, setReaderInit)
	ZX_PROPERTY(std::shared_ptr<CustomData>, extra, setExtra)

#undef ZX_PROPERTY
};

} // ZXing
