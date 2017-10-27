#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <string>

namespace ZXing {

enum class CharacterSet;
class BitMatrix;

namespace Aztec {

class Writer
{
public:
	Writer();

	Writer& setEncoding(CharacterSet encoding) {
		_encoding = encoding;
		return *this;
	}

	Writer& setEccPercent(int percent) {
		_eccPercent = percent;
		return *this;
	}

	Writer& setLayers(int layers) {
		_layers = layers;
		return *this;
	}

	BitMatrix encode(const std::wstring& contents, int width, int height) const;

private:
	CharacterSet _encoding;
	int _eccPercent;
	int _layers;
};

} // Aztec
} // ZXing
