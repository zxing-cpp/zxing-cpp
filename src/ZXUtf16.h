#pragma once
/*
* Copyright 2016 Huy Cuong Nguyen
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
#include <vector>

namespace ZXing {

class Utf16
{
public:
	template <typename T>
	static bool IsHighSurrogate(T c)
	{
		return (c & 0xfc00) == 0xd800;
	}
	
	template <typename T>
	static bool IsLowSurrogate(T c)
	{
		return (c & 0xfc00) == 0xdc00;
	}
	
	template <typename T>
	static uint32_t CodePointFromSurrogates(T high, T low)
	{
		return (uint32_t(high) << 10) + low - 0x35fdc00;
	}

	static bool RequiresSurrogates(uint32_t ucs4)
	{
		return ucs4 >= 0x10000;
	}

	static uint16_t HighSurrogate(uint32_t ucs4)
	{
		return uint16_t((ucs4 >> 10) + 0xd7c0);
	}

	static uint16_t LowSurrogate(uint32_t ucs4)
	{
		return uint16_t(ucs4 % 0x400 + 0xdc00);
	}

	template<typename T>
	static int Utf16toUtf32(const uint16_t *utf16, int length, T *out)
	{
		int i = 0;
		for (; i < length; ++i)
		{
			unsigned u = utf16[i];
			if (IsHighSurrogate(u) && i < length-1)
			{
				unsigned low = utf16[i+1];
				if (IsLowSurrogate(low))
				{
					++i;
					u = CodePointFromSurrogates(u, low);
				}
			}
			*out++ = T(u);
		}
		return i;
	}

	static void Utf16toUtf32(const std::vector<uint16_t>& utf16, std::vector<uint32_t>& utf32)
	{
		utf32.resize(utf16.size());
		int len = Utf16toUtf32(&utf16[0], static_cast<int>(utf16.size()), &utf32[0]);
		utf32.resize(len);
	}

	static std::vector<uint32_t> Utf16toUtf32(const std::vector<uint16_t>& utf16)
	{
		std::vector<uint32_t> utf32;
		utf32.resize(utf16.size());
		int len = Utf16toUtf32(&utf16[0], static_cast<int>(utf16.size()), &utf32[0]);
		utf32.resize(len);
		return utf32;
	}

	static std::vector<uint16_t> Utf32toUtf16(const std::vector<uint32_t>& utf32)
	{
		std::vector<uint16_t> result;
		result.reserve(utf32.size());
		for (size_t i = 0; i < utf32.size(); ++i)
		{
			uint32_t c = utf32[i];
			if (RequiresSurrogates(c))
			{
				result.push_back(HighSurrogate(c));
				result.push_back(LowSurrogate(c));
			}
			else
			{
				result.push_back(c);
			}
		}
		return result;
	}

};

} // ZXing
