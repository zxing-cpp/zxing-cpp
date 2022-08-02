/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#include "TextUtfEncoding.h"

#include <locale>
#include <iomanip>
#include <sstream>

namespace ZXing::TextUtfEncoding {

static size_t Utf8CountCodePoints(const uint8_t* utf8, size_t length)
{
	size_t count = 0;

	for (size_t i = 0; i < length;) {
		if (utf8[i] < 128) {
			++i;
		} else {
			switch (utf8[i] & 0xf0) {
			case 0xc0: [[fallthrough]];
			case 0xd0: i += 2; break;
			case 0xe0: i += 3; break;
			case 0xf0: i += 4; break;
			default: // we are in middle of a sequence
				++i;
				while (i < length && (utf8[i] & 0xc0) == 0x80)
					++i;
				break;
			}
		}
		++count;
	}
	return count;
}

constexpr uint32_t kAccepted = 0;
constexpr uint32_t kRejected [[maybe_unused]] = 12;

inline uint32_t Utf8Decode(uint8_t byte, uint32_t& state, uint32_t& codep)
{
	// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	static const uint8_t kUtf8Data[] = {
		/* The first part of the table maps bytes to character classes that
		 * reduce the size of the transition table and create bitmasks. */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
		10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

		/* The second part is a transition table that maps a combination
		 * of a state of the automaton and a character class to a state. */
		0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
		12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
		12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
		12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
		12,36,12,12,12,12,12,12,12,12,12,12,
	};

	uint32_t type = kUtf8Data[byte];
	codep = (state != kAccepted) ? (byte & 0x3fu) | (codep << 6) : (0xff >> type) & (byte);
	state = kUtf8Data[256 + state + type];
	return state;
}

static_assert(sizeof(wchar_t) == 4 || sizeof(wchar_t) == 2, "wchar_t needs to be 2 or 4 bytes wide");

static void ConvertFromUtf8(const uint8_t* src, size_t length, std::wstring& buffer)
{
	size_t destLen = Utf8CountCodePoints(src, length);

	buffer.reserve(buffer.size() + destLen);
	uint32_t codePoint = 0;
	uint32_t state = kAccepted;

	for (auto i = src, end = src + length; i < end; ++i) {
		if (Utf8Decode(*i, state, codePoint) != kAccepted)
			continue;

		if (sizeof(wchar_t) == 2 && codePoint > 0xffff) { // surrogate pair
			buffer.push_back(static_cast<wchar_t>(0xd7c0 + (codePoint >> 10)));
			buffer.push_back(static_cast<wchar_t>(0xdc00 + (codePoint & 0x3ff)));
		} else
			buffer.push_back(static_cast<wchar_t>(codePoint));
	}
}

/// <summary>
/// Count the number of bytes required to store given code points in UTF-8.
/// </summary>
static size_t Utf8CountBytes(const wchar_t* str, size_t length)
{
	int result = 0;
	for (size_t i = 0; i < length; ++i) {
		if (str[i] < 0x80)
			result += 1;
		else if (str[i] < 0x800)
			result += 2;
		else if (sizeof(wchar_t) == 4) {
			if (str[i] < 0x10000)
				result += 3;
			else
				result += 4;
		} else {
			if (IsUtf16HighSurrogate(str[i])) {
				result += 4;
				++i;
			} else
				result += 3;
		}
	}
	return result;
}

static int Utf8Encode(uint32_t utf32, char* out)
{
	if (utf32 < 0x80) {
		*out++ = static_cast<uint8_t>(utf32);
		return 1;
	}
	if (utf32 < 0x800) {
		*out++ = static_cast<uint8_t>((utf32 >> 6) | 0xc0);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f) | 0x80);
		return 2;
	}
	if (utf32 < 0x10000) {
		*out++ = static_cast<uint8_t>((utf32 >> 12) | 0xe0);
		*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f) | 0x80);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f) | 0x80);
		return 3;
	}

	*out++ = static_cast<uint8_t>((utf32 >> 18) | 0xf0);
	*out++ = static_cast<uint8_t>(((utf32 >> 12) & 0x3f) | 0x80);
	*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f) | 0x80);
	*out++ = static_cast<uint8_t>((utf32 & 0x3f) | 0x80);
	return 4;
}

static void ConvertToUtf8(const std::wstring& str, std::string& utf8)
{
	char buffer[4];
	for (size_t i = 0; i < str.length(); ++i)
	{
		uint32_t c;
		if (sizeof(wchar_t) == 2 && i + 1 < str.length() && IsUtf16HighSurrogate(str[i]) &&
			IsUtf16LowSurrogate(str[i + 1])) {
			c = CodePointFromUtf16Surrogates(str[i], str[i + 1]);
			++i;
		} else
			c = str[i];

		auto bufLength = Utf8Encode(c, buffer);
		utf8.append(buffer, bufLength);
	}
}

void ToUtf8(const std::wstring& str, std::string& utf8)
{
	utf8.reserve(str.length() + Utf8CountBytes(str.data(), str.length()));
	ConvertToUtf8(str, utf8);
}

std::wstring FromUtf8(const std::string& utf8)
{
	std::wstring str;
	ConvertFromUtf8(reinterpret_cast<const uint8_t*>(utf8.data()), utf8.length(), str);
	return str;
}

std::string ToUtf8(const std::wstring& str)
{
	std::string utf8;
	ToUtf8(str, utf8);
	return utf8;
}

// Same as `ToUtf8()` above, except if angleEscape set, places non-graphical characters in angle brackets with text name
std::string ToUtf8(const std::wstring& str, const bool angleEscape)
{
	if (!angleEscape) {
		return ToUtf8(str);
	}
	static const char* const ascii_nongraphs[33] = {
		"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
		 "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
		"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
		"CAN",  "EM", "SUB", "ESC",  "FS",  "GS",  "RS",  "US",
		"DEL",
	};

	std::locale utf8Loc("en_US.UTF-8");

	std::wostringstream ws;
	ws.fill(L'0');

	for (unsigned int i = 0; i < str.length(); i++) {
		wchar_t wc = str[i];
		if (wc < 128) { // ASCII
			if (wc < 32 || wc == 127) { // Non-graphical ASCII, excluding space
				ws << "<" << ascii_nongraphs[wc == 127 ? 32 : wc] << ">";
			} else {
				ws << wc;
			}
		} else {
			// Surrogates (Windows) need special treatment
			if (i + 1 < str.length() && IsUtf16HighSurrogate(wc) && IsUtf16LowSurrogate(str[i + 1])) {
				ws.write(str.c_str() + i++, 2);
			} else {
				// Exclude unpaired surrogates and NO-BREAK spaces NBSP and NUMSP
				if ((wc < 0xd800 || wc >= 0xe000) && (std::isgraph(wc, utf8Loc) && wc != 0xA0 && wc != 0x2007 && wc != 0xfffd)) {
					ws << wc;
				} else { // Non-graphical Unicode
					int width = wc < 256 ? 2 : 4;
					ws << "<U+" << std::setw(width) << std::uppercase << std::hex
					   << static_cast<unsigned int>(wc) << ">";
				}
			}
		}
	}

	return ToUtf8(ws.str());
}

void AppendUtf16(std::wstring& str, const uint16_t* utf16, size_t length)
{
	if (sizeof(wchar_t) == 2) {
		str.append(reinterpret_cast<const wchar_t*>(utf16), length);
	}
	else {
		str.reserve(str.length() + length);
		for (size_t i = 0; i < length; ++i)
		{
			unsigned u = utf16[i];
			if (IsUtf16HighSurrogate(u) && i + 1 < length)
			{
				unsigned low = utf16[i + 1];
				if (IsUtf16LowSurrogate(low))
				{
					++i;
					u = CodePointFromUtf16Surrogates(u, low);
				}
			}
			str.push_back(static_cast<wchar_t>(u));
		}
	}
}

void AppendUtf8(std::wstring& str, const uint8_t* utf8, size_t length)
{
	ConvertFromUtf8(utf8, length, str);
}

} // namespace ZXing::TextUtfEncoding
