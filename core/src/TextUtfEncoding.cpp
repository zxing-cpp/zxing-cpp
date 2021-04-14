/*
* Copyright 2016 Nu-book Inc.
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

#include "TextUtfEncoding.h"

#include <cwctype>
#include <iomanip>
#include <sstream>
#include <type_traits>

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

static const uint32_t kAccepted = 0;

/// <summary>
/// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
/// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
/// </summary>
static uint32_t Utf8Decode(uint8_t byte, uint32_t& state, uint32_t& codep)
{
	// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	static const uint8_t kUtf8Data[] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
		8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
		0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
		0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
		0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
		1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
		1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
		1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
	};

	uint32_t type = kUtf8Data[byte];
	codep = (state != kAccepted) ? (byte & 0x3fu) | (codep << 6) : (0xff >> type) & (byte);
	state = kUtf8Data[256 + state * 16 + type];
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
// Note `std::setlocale(LC_CTYPE, "en_US.UTF-8")` must be set beforehand for `std::iswraph()` to work
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
				if ((wc < 0xd800 || wc >= 0xe000) && (std::iswgraph(wc) && wc != 0xA0 && wc != 0x2007)) {
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
