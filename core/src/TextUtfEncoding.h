#pragma once
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

#include <cstdint>
#include <cstddef>
#include <string>

namespace ZXing {

class TextUtfEncoding
{
public:
	static std::string ToUtf8(const std::wstring& str);
	static std::wstring FromUtf8(const std::string& utf8);

	static void ToUtf8(const std::wstring& str, std::string& utf8);
	static void AppendUtf16(std::wstring& str, const uint16_t* utf16, size_t length);
	static void AppendUtf8(std::wstring& str, const uint8_t* utf8, size_t length);

	template <typename T>
	static bool IsUtf16HighSurrogate(T c)
	{
		return (c & 0xfc00) == 0xd800;
	}

	template <typename T>
	static bool IsUtf16LowSurrogate(T c)
	{
		return (c & 0xfc00) == 0xdc00;
	}

	template <typename T>
	static uint32_t CodePointFromUtf16Surrogates(T high, T low)
	{
		return (uint32_t(high) << 10) + low - 0x35fdc00;
	}
};

} // ZXing
