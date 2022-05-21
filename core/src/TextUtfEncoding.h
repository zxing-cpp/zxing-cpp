/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ZXing {
namespace TextUtfEncoding {

std::string ToUtf8(const std::wstring& str);
std::string ToUtf8(const std::wstring& str, const bool angleEscape);
std::wstring FromUtf8(const std::string& utf8);

void ToUtf8(const std::wstring& str, std::string& utf8);
void AppendUtf16(std::wstring& str, const uint16_t* utf16, size_t length);
void AppendUtf8(std::wstring& str, const uint8_t* utf8, size_t length);

template <typename T>
bool IsUtf16HighSurrogate(T c)
{
	return (c & 0xfc00) == 0xd800;
}

template <typename T>
bool IsUtf16LowSurrogate(T c)
{
	return (c & 0xfc00) == 0xdc00;
}

template <typename T>
uint32_t CodePointFromUtf16Surrogates(T high, T low)
{
	return (uint32_t(high) << 10) + low - 0x35fdc00;
}

} // namespace TextUtfEncoding
} // namespace ZXing
