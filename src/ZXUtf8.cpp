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

#include "ZXUtf8.h"

namespace ZXing {

namespace {

	// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
	// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	const uint8_t kUtf8Data[] =
	{
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

} // anonymous

// George Pollard:
// http://porg.es/blog/counting-characters-in-utf-8-strings-is-faster
int
Utf8::CountCodePoints(const char *utf8)
{
	int i = 0;
	int ibefore = 0;
	int count = 0;
	const signed char *s = (const signed char *)utf8;

	/* using signed chars so ascii bytes are positive */
	while (s[i] > 0) {
	ascii:
		i++;
	}

	count += (i - ibefore);
	while (s[i])
	{
		if (s[i] > 0)
		{
			ibefore = i;
			goto ascii;
		}
		else
		{
			switch (s[i] & 0xf0)
			{
			case 0xc0: case 0xd0: i += 2; break;
			case 0xe0: i += 3; break;
			case 0xf0: i += 4; break;
			default: // we are in middle of a sequence
				while ((s[++i] & 0xc0) == 0x80); break; 
			}
		}
		++count;
	}
	return count;
}

const char *
Utf8::SkipCodePoints(const char* utf8, int count)
{
	int i = 0;
	const signed char *s = (const signed char *)utf8;
	while (s[i] && count > 0)
	{
		if (s[i] > 0)
		{
			i++;
		}
		else
		{
			switch (s[i] & 0xf0)
			{
			case 0xc0: case 0xd0: i += 2; break;
			case 0xe0: i += 3; break;
			case 0xf0: i += 4; break;
			default: // we are in middle of a sequence
				while ((s[++i] & 0xc0) == 0x80); break;
			}
		}
		--count;
	}
	return (const char *)s + i;
}

/// <summary>
/// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
/// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
/// </summary>
uint32_t
Utf8::Decode(uint8_t byte, uint32_t& state, uint32_t& codep)
{
	uint32_t type = kUtf8Data[byte];
	codep = (state != kAccepted) ? (byte & 0x3fu) | (codep << 6) : (0xff >> type) & (byte);
	state = kUtf8Data[256 + state*16 + type];
	return state;
}

int
Utf8::Encode(uint32_t utf32, char* out)
{
	if (utf32 < 0x80)
	{
		*out++ = static_cast<uint8_t>(utf32);
		return 1;
	}
	if (utf32 < 0x800)
	{
		*out++ = static_cast<uint8_t>((utf32 >> 6)			| 0xc0);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f)		| 0x80);
		return 2;
	}
	if (utf32 < 0x10000)
	{
		*out++ = static_cast<uint8_t>((utf32 >> 12)			| 0xe0);
		*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f)	| 0x80);
		*out++ = static_cast<uint8_t>((utf32 & 0x3f)		| 0x80);
		return 3;
	}

	*out++ = static_cast<uint8_t>((utf32 >> 18)				| 0xf0);
	*out++ = static_cast<uint8_t>(((utf32 >> 12) & 0x3f)	| 0x80);
	*out++ = static_cast<uint8_t>(((utf32 >> 6) & 0x3f)		| 0x80);
	*out++ = static_cast<uint8_t>((utf32 & 0x3f)			| 0x80);
	return 4;
}

} // ZXing
