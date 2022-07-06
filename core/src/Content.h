/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"

#include <string>
#include <vector>

namespace ZXing {

enum class ECI : int;
enum class CharacterSet;

enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };
enum class TextMode { Utf8, Utf8ECI, HRI, Hex, Escaped };

std::string ToString(ContentType type);

struct SymbologyIdentifier
{
	char code = 0, modifier = 0;
	int eciModifierOffset = 0;

	std::string toString(bool hasECI = false) const
	{
		return code ? ']' + std::string(1, code) + static_cast<char>(modifier + eciModifierOffset * hasECI) : std::string();
	}
};

class Content
{
	template <typename FUNC>
	void ForEachECIBlock(FUNC f) const;

	void switchEncoding(ECI eci, bool isECI);
	std::wstring render(bool withECI) const;

public:
	struct Encoding
	{
		ECI eci;
		int pos;
	};

	ByteArray bytes;
	std::vector<Encoding> encodings;
	std::string defaultCharset;
	std::string applicationIndicator;
	SymbologyIdentifier symbology;
	bool hasECI = false;

	Content();
	Content(ByteArray&& bytes, SymbologyIdentifier si, std::string ai = {});

	void switchEncoding(ECI eci) { switchEncoding(eci, true); }
	void switchEncoding(CharacterSet cs);

	void reserve(int count) { bytes.reserve(bytes.size() + count); }

	void push_back(uint8_t val) { bytes.push_back(val); }
	void append(const std::string& str) { bytes.insert(bytes.end(), str.begin(), str.end()); }
	void append(const ByteArray& ba) { bytes.insert(bytes.end(), ba.begin(), ba.end()); }
	void append(const Content& other);

	void operator+=(char val) { push_back(val); }
	void operator+=(const std::string& str) { append(str); }

	void erase(int pos, int n);
	void insert(int pos, const std::string& str);

	bool empty() const { return bytes.empty(); }
	bool canProcess() const;

	std::string text(TextMode mode) const;
	std::wstring utf16() const { return render(false); }
	std::string utf8() const { return text(TextMode::Utf8); }

	ByteArray bytesECI() const;
	CharacterSet guessEncoding() const;
	ContentType type() const;
};

} // ZXing
