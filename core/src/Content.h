#pragma once
/*
* Copyright 2022 Axel Waggershauser
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
#include "CharacterSet.h"

namespace ZXing {

enum class ContentType { Text, Binary, Mixed };

class Content
{
	bool hasECI = false;

public:
	ByteArray binary;
	using Encoding = std::pair<CharacterSet, int>;
	std::vector<Encoding> encodings = {{CharacterSet::Unknown, 0}};
	std::string hintedCharset;

	void switchEncoding(CharacterSet cs, bool isECI = false);

	void reserve(int count) { binary.reserve(binary.size() + count); }

	void push_back(uint8_t val) { binary.push_back(val); }
	void append(const std::string& str) { binary.insert(binary.end(), str.begin(), str.end()); }
	void append(const ByteArray& bytes) { binary.insert(binary.end(), bytes.begin(), bytes.end()); }

	void operator+=(char val) { push_back(val); }
	void operator+=(const std::string& str) { append(str); }

	bool empty() const { return binary.empty(); }

	std::wstring text() const;
	CharacterSet guessEncoding() const;
	ContentType type() const;
};

} // ZXing
