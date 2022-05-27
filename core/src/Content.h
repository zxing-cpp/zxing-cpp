/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"
#include "ECI.h"

namespace ZXing {

enum class ContentType { Text, Binary, Mixed };

class Content
{
	bool hasECI = false;

	template <typename FUNC>
	void ForEachECIBlock(FUNC f) const;

	void switchEncoding(ECI eci, bool isECI);

public:
	struct Encoding
	{
		ECI eci;
		int pos;
	};

	ByteArray binary;
	std::vector<Encoding> encodings = {{ECI::Unknown, 0}};
	std::string hintedCharset;
	std::string applicationIndicator;

	Content() = default;
	Content(ByteArray&& binary) : binary(std::move(binary)), encodings{{ECI::ISO8859_1, 0}} {}

	void switchEncoding(ECI eci) { switchEncoding(eci, true); }
	void switchEncoding(CharacterSet cs);

	void reserve(int count) { binary.reserve(binary.size() + count); }

	void push_back(uint8_t val) { binary.push_back(val); }
	void append(const std::string& str) { binary.insert(binary.end(), str.begin(), str.end()); }
	void append(const ByteArray& bytes) { binary.insert(binary.end(), bytes.begin(), bytes.end()); }

	void operator+=(char val) { push_back(val); }
	void operator+=(const std::string& str) { append(str); }

	bool empty() const { return binary.empty(); }

	std::wstring text() const;
	std::string utf8Protocol() const;
	CharacterSet guessEncoding() const;
	ContentType type() const;
};

} // ZXing
