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

namespace ZXing {

static size_t Utf8CountCodePoints(const uint8_t *utf8, size_t length)
{
	size_t i = 0;
	size_t count = 0;

	while (i < length) {
		if (utf8[i] < 128) {
			++i;
		}
		else {
			switch (utf8[i] & 0xf0) {
			case 0xc0:
			case 0xd0:
				i += 2; break;
			case 0xe0:
				i += 3; break;
			case 0xf0:
				i += 4; break;
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

template <typename WCharT>
static void ConvertFromUtf8(const uint8_t* src, size_t length, std::basic_string<WCharT>& buffer, typename std::enable_if<(sizeof(WCharT) == 2)>::type* = nullptr)
{
	const uint8_t* srcEnd = src + length;
	size_t destLen = Utf8CountCodePoints(src, length);
	if (destLen > 0) {
		buffer.reserve(buffer.size() + destLen);
		uint32_t codePoint = 0;
		uint32_t state = kAccepted;

		while (src < srcEnd) {
			if (Utf8Decode(*src++, state, codePoint) != kAccepted) {
				continue;
			}
			if (codePoint > 0xffff) { // surrogate pair
				buffer.push_back((WCharT)(0xd7c0 + (codePoint >> 10)));
				buffer.push_back((WCharT)(0xdc00 + (codePoint & 0x3ff)));
			}
			else {
				buffer.push_back((WCharT)codePoint);
			}
		}
	}
}

template <typename WCharT>
static void ConvertFromUtf8(const uint8_t* src, size_t length, std::basic_string<WCharT>& buffer, typename std::enable_if<(sizeof(WCharT) == 4)>::type* = nullptr)
{
	const uint8_t* srcEnd = src + length;
	size_t destLen = Utf8CountCodePoints(src, length);
	if (destLen > 0) {
		buffer.reserve(buffer.size() + destLen);
		uint32_t codePoint = 0;
		uint32_t state = kAccepted;

		while (src < srcEnd) {
			if (Utf8Decode(*src++, state, codePoint) != kAccepted) {
				continue;
			}
			buffer.push_back((WCharT)codePoint);
		}
	}
}


/// <summary>
/// Count the number of bytes required to store given code points in UTF-8.
/// </summary>
template <typename WCharT>
static size_t Utf8CountBytes(const WCharT* utf32, size_t length, typename std::enable_if<(sizeof(WCharT) == 4)>::type* = nullptr)
{
	int result = 0;
	for (size_t i = 0; i < length; ++i) {
		unsigned codePoint = (unsigned)utf32[i];
		if (codePoint < 0x80)
		{
			result += 1;
		}
		else if (codePoint < 0x800)
		{
			result += 2;
		}
		else if (codePoint < 0x10000)
		{
			result += 3;
		}
		else
		{
			result += 4;
		}
	}
	return result;
}

template <typename WCharT>
static size_t Utf8CountBytes(const WCharT* utf16, size_t length, typename std::enable_if<(sizeof(WCharT) == 2)>::type* = nullptr)
{
	int result = 0;
	for (size_t i = 0; i < length; ++i) {
		uint16_t codePoint = utf16[i];
		if (codePoint < 0x80)
		{
			result += 1;
		}
		else if (codePoint < 0x800)
		{
			result += 2;
		}
		else if (TextUtfEncoding::IsUtf16HighSurrogate(codePoint))
		{
			result += 4;
			++i;
		}
		else
		{
			result += 3;
		}
	}
	return result;
}

static int Utf8Encode(uint32_t utf32, char* out)
{
	if (utf32 < 0x80)
	{
		*out++ = static_cast<uint8_t>(utf32);
		return 1;
	}
	if (utf32 < 0x800)
	{
		*out++ = static_cast<uint8_t>((utf32 >> 6) | 0xc0);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f) | 0x80);
		return 2;
	}
	if (utf32 < 0x10000)
	{
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

template <typename WCharT>
static void ConvertToUtf8(const std::basic_string<WCharT>& str, std::string& utf8, typename std::enable_if<(sizeof(WCharT) == 2)>::type* = nullptr)
{
	char buffer[4];
	int bufLength;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (i + 1 < str.length() && TextUtfEncoding::IsUtf16HighSurrogate(str[i]) && TextUtfEncoding::IsUtf16LowSurrogate(str[i + 1]))
		{
			bufLength = Utf8Encode(TextUtfEncoding::CodePointFromUtf16Surrogates(str[i], str[i + 1]), buffer);
			++i;
		}
		else
		{
			bufLength = Utf8Encode(str[i], buffer);
		}
		utf8.append(buffer, bufLength);
	}
}

template <typename WCharT>
static void ConvertToUtf8(const std::basic_string<WCharT>& str, std::string& utf8, typename std::enable_if<(sizeof(WCharT) == 4)>::type* = nullptr)
{
	char buffer[4];
	for (auto c : str) {
		auto bufLength = Utf8Encode(c, buffer);
		utf8.append(buffer, bufLength);
	}
}


void
TextUtfEncoding::ToUtf8(const std::wstring& str, std::string& utf8)
{
	utf8.reserve(str.length() + Utf8CountBytes(str.data(), str.length()));
	ConvertToUtf8(str, utf8);
}

std::wstring
TextUtfEncoding::FromUtf8(const std::string& utf8)
{
	std::wstring str;
	ConvertFromUtf8(reinterpret_cast<const uint8_t*>(utf8.data()), utf8.length(), str);
	return str;
}

std::string
TextUtfEncoding::ToUtf8(const std::wstring& str)
{
	std::string utf8;
	ToUtf8(str, utf8);
	return utf8;
}

void
TextUtfEncoding::AppendUtf16(std::wstring& str, const uint16_t* utf16, size_t length)
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

void
TextUtfEncoding::AppendUtf8(std::wstring& str, const uint8_t* utf8, size_t length)
{
	ConvertFromUtf8(utf8, length, str);
}


} // ZXing
