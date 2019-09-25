#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
* Copyright 2006-2007 Jeremias Maerki.
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
#include <string>
#include <utility>
#include <stdexcept>
#include "ByteArray.h"
#include "DMSymbolShape.h"
#include "DMSymbolInfo.h"
#include "ZXStrConvWorkaround.h"

namespace ZXing {
namespace DataMatrix {

class SymbolInfo;

class EncoderContext
{
	std::string _msg;
	SymbolShape _shape = SymbolShape::NONE;
	int _minWidth = -1;
	int _minHeight = -1;
	int _maxWidth = -1;
	int _maxHeight = -1;
	ByteArray _codewords;
	int _pos = 0;
	int _newEncoding = -1;
	const SymbolInfo* _symbolInfo = nullptr;
	int _skipAtEnd = 0;

public:
	explicit EncoderContext(std::string&& msg) : _msg(std::move(msg)) { _codewords.reserve(_msg.length()); }
	
	EncoderContext(const EncoderContext &) = delete;	// avoid copy by mistake

	void setSymbolShape(SymbolShape shape) {
		_shape = shape;
	}

	void setSizeConstraints(int minWidth, int minHeight, int maxWidth, int maxHeight) {
		_minWidth = minWidth;
		_minHeight = minHeight;
		_maxWidth = maxWidth;
		_maxHeight = maxHeight;
	}

	const std::string& message() const {
		return _msg;
	}

	void setSkipAtEnd(int count) {
		_skipAtEnd = count;
	}

	int currentPos() const {
		return _pos;
	}

	void setCurrentPos(int pos) {
		_pos = pos;
	}

	int currentChar() const {
		return _msg.at(_pos) & 0xff;
	}

	int nextChar() const {
		return _msg.at(_pos + 1) & 0xff;
	}

	const ByteArray& codewords() const {
		return _codewords;
	}

	int codewordCount() const {
		return (int)_codewords.size();
	}

	void addCodeword(uint8_t codeword) {
		_codewords.push_back(codeword);
	}

	void setNewEncoding(int encoding) {
		_newEncoding = encoding;
	}

	void clearNewEncoding() {
		_newEncoding = -1;
	}

	int newEncoding() const {
		return _newEncoding;
	}

	bool hasMoreCharacters() const {
		return _pos < totalMessageCharCount();
	}

	int totalMessageCharCount() const {
		return static_cast<int>(_msg.length() - _skipAtEnd);
	}

	int remainingCharacters() const {
		return totalMessageCharCount() - _pos;
	}

	const SymbolInfo* updateSymbolInfo(int len) {
		if (_symbolInfo == nullptr || len > _symbolInfo->dataCapacity()) {
			_symbolInfo = SymbolInfo::Lookup(len, _shape, _minWidth, _minHeight, _maxWidth, _maxHeight);
			if (_symbolInfo == nullptr) {
				throw std::invalid_argument("Can't find a symbol arrangement that matches the message. Data codewords: " + std::to_string(len));
			}
		}
		return _symbolInfo;
	}

	void resetSymbolInfo() {
		_symbolInfo = nullptr;
	}
	
	const SymbolInfo* symbolInfo() const {
		return _symbolInfo;
	}
};

} // DataMatrix
} // ZXing
