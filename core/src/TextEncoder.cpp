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

#include "TextEncoder.h"
#include "CharacterSet.h"
#include "TextUtfEncoding.h"
#include "textcodec/JPTextEncoder.h"
#include "textcodec/Big5TextEncoder.h"
#include "textcodec/GBTextEncoder.h"
#include "textcodec/KRTextEncoder.h"
#include "ZXContainerAlgorithms.h"

#include <stdexcept>
#include <algorithm>

namespace ZXing {

namespace {

struct MapEntry
{
	uint16_t unicode;
	uint8_t count;
	uint8_t charcode;
};

inline bool operator<(const uint16_t& val, const MapEntry& b) { return val < b.unicode; }

static const MapEntry latin2Mapping[45] = {
    {0x0080,33,  0}, {0x00a4, 1, 36}, {0x00a7, 2, 39}, {0x00ad, 1, 45}, {0x00b0, 1, 48}, {0x00b4, 1, 52}, {0x00b8, 1, 56}, {0x00c1, 2, 65},
    {0x00c4, 1, 68}, {0x00c7, 1, 71}, {0x00c9, 1, 73}, {0x00cb, 1, 75}, {0x00cd, 2, 77}, {0x00d3, 2, 83}, {0x00d6, 2, 86}, {0x00da, 1, 90},
    {0x00dc, 2, 92}, {0x00df, 1, 95}, {0x00e1, 2, 97}, {0x00e4, 1,100}, {0x00e7, 1,103}, {0x00e9, 1,105}, {0x00eb, 1,107}, {0x00ed, 2,109},
    {0x00f3, 2,115}, {0x00f6, 2,118}, {0x00fa, 1,122}, {0x00fc, 2,124}, {0x0102, 6, 67}, {0x010c, 6, 72}, {0x0118, 4, 74}, {0x0139, 2, 69},
    {0x013d, 2, 37}, {0x0141, 4, 35}, {0x0147, 2, 82}, {0x0150, 2, 85}, {0x0154, 2, 64}, {0x0158, 4, 88}, {0x015e, 8, 42}, {0x016e, 4, 89},
    {0x0179, 6, 44}, {0x02c7, 1, 55}, {0x02d8, 2, 34}, {0x02db, 1, 50}, {0x02dd, 1, 61},
};

static const MapEntry latin3Mapping[30] = {
    {0x0080,33,  0}, {0x00a3, 2, 35}, {0x00a7, 2, 39}, {0x00ad, 1, 45}, {0x00b0, 1, 48}, {0x00b2, 4, 50}, {0x00b7, 2, 55}, {0x00bd, 1, 61},
    {0x00c0, 3, 64}, {0x00c4, 1, 68}, {0x00c7, 9, 71}, {0x00d1, 4, 81}, {0x00d6, 2, 86}, {0x00d9, 4, 89}, {0x00df, 4, 95}, {0x00e4, 1,100},
    {0x00e7, 9,103}, {0x00f1, 4,113}, {0x00f6, 2,118}, {0x00f9, 4,121}, {0x0108, 4, 70}, {0x011c, 6, 88}, {0x0124, 4, 38}, {0x0130, 2, 41},
    {0x0134, 2, 44}, {0x015c, 4, 94}, {0x016c, 2, 93}, {0x017b, 2, 47}, {0x02d8, 2, 34}, {0xfffd, 1,112},
};

static const MapEntry latin4Mapping[40] = {
    {0x0080,33,  0}, {0x00a4, 1, 36}, {0x00a7, 2, 39}, {0x00ad, 1, 45}, {0x00af, 2, 47}, {0x00b4, 1, 52}, {0x00b8, 1, 56}, {0x00c1, 6, 65},
    {0x00c9, 1, 73}, {0x00cb, 1, 75}, {0x00cd, 2, 77}, {0x00d4, 5, 84}, {0x00da, 3, 90}, {0x00df, 1, 95}, {0x00e1, 6, 97}, {0x00e9, 1,105},
    {0x00eb, 1,107}, {0x00ed, 2,109}, {0x00f4, 5,116}, {0x00fa, 3,122}, {0x0100, 2, 64}, {0x0104, 2, 33}, {0x010c, 2, 72}, {0x0110, 4, 80},
    {0x0116, 4, 76}, {0x0122, 2, 43}, {0x0128, 4, 37}, {0x012e, 2, 71}, {0x0136, 3, 83}, {0x013b, 2, 38}, {0x0145, 2, 81}, {0x014a, 4, 61},
    {0x0156, 2, 35}, {0x0160, 2, 41}, {0x0166, 6, 44}, {0x0172, 2, 89}, {0x017d, 2, 46}, {0x02c7, 1, 55}, {0x02d9, 1,127}, {0x02db, 1, 50},
};

static const MapEntry latin5Mapping[8] = {
    {0x0080,33,  0}, {0x00a7, 1,125}, {0x00ad, 1, 45}, {0x0401,12, 33}, {0x040e,66, 46}, {0x0451,12,113}, {0x045e, 2,126}, {0x2116, 1,112},
};

static const MapEntry latin6Mapping[9] = {
    {0x0080,33,  0}, {0x00a4, 1, 36}, {0x00ad, 1, 45}, {0x060c, 1, 44}, {0x061b, 1, 59}, {0x061f, 1, 63}, {0x0621,26, 65}, {0x0640,19, 96},
    {0xfffd, 1,127},
};

static const MapEntry latin7Mapping[16] = {
    {0x0080,33,  0}, {0x00a3, 1, 35}, {0x00a6, 4, 38}, {0x00ab, 3, 43}, {0x00b0, 4, 48}, {0x00b7, 1, 55}, {0x00bb, 1, 59}, {0x00bd, 1, 61},
    {0x0384, 3, 52}, {0x0388, 3, 56}, {0x038c, 1, 60}, {0x038e,20, 62}, {0x03a3,44, 83}, {0x2015, 1, 47}, {0x2018, 2, 33}, {0xfffd, 1,127},
};

static const MapEntry latin8Mapping[11] = {
    {0x0080,33,  0}, {0x00a2, 8, 34}, {0x00ab, 4, 43}, {0x00b0,10, 48}, {0x00bb, 4, 59}, {0x00d7, 1, 42}, {0x00f7, 1, 58}, {0x05d0,27, 96},
    {0x2017, 1, 95}, {0x203e, 1, 47}, {0xfffd, 1,127},
};

static const MapEntry latin9Mapping[8] = {
    {0x0080,80,  0}, {0x00d1,12, 81}, {0x00df,17, 95}, {0x00f1,12,113}, {0x00ff, 1,127}, {0x011e, 2, 80}, {0x0130, 2, 93}, {0x015e, 2, 94},
};

static const MapEntry latin10Mapping[36] = {
    {0x0080,33,  0}, {0x00a7, 1, 39}, {0x00ad, 1, 45}, {0x00b0, 1, 48}, {0x00b7, 1, 55}, {0x00c1, 6, 65}, {0x00c9, 1, 73}, {0x00cb, 1, 75},
    {0x00cd, 4, 77}, {0x00d3, 4, 83}, {0x00d8, 1, 88}, {0x00da, 6, 90}, {0x00e1, 6, 97}, {0x00e9, 1,105}, {0x00eb, 1,107}, {0x00ed, 4,109},
    {0x00f3, 4,115}, {0x00f8, 1,120}, {0x00fa, 5,122}, {0x0100, 2, 64}, {0x0104, 2, 33}, {0x010c, 2, 72}, {0x0110, 4, 41}, {0x0116, 4, 76},
    {0x0122, 2, 35}, {0x0128, 4, 37}, {0x012e, 2, 71}, {0x0136, 3, 38}, {0x013b, 2, 40}, {0x0145, 2, 81}, {0x014a, 4, 47}, {0x0160, 2, 42},
    {0x0166, 6, 43}, {0x0172, 2, 89}, {0x017d, 2, 44}, {0x2015, 1, 61},
};

static const MapEntry latin11Mapping[9] = {
    {0x0e01,58, 33}, {0x0e3f,29, 95}, {0x2013, 2, 22}, {0x2018, 2, 17}, {0x201c, 2, 19}, {0x2022, 1, 21}, {0x2026, 1,  5}, {0x20ac, 1,  0},
    {0xfffd, 1,127},
};

static const MapEntry latin13Mapping[40] = {
    {0x0080,33,  0}, {0x00a2, 3, 34}, {0x00a6, 2, 38}, {0x00a9, 1, 41}, {0x00ab, 4, 43}, {0x00b0, 4, 48}, {0x00b5, 3, 53}, {0x00b9, 1, 57},
    {0x00bb, 4, 59}, {0x00c4, 3, 68}, {0x00c9, 1, 73}, {0x00d3, 1, 83}, {0x00d5, 4, 85}, {0x00dc, 1, 92}, {0x00df, 1, 95}, {0x00e4, 3,100},
    {0x00e9, 1,105}, {0x00f3, 1,115}, {0x00f5, 4,117}, {0x00fc, 1,124}, {0x0100, 2, 66}, {0x0104, 4, 64}, {0x010c, 2, 72}, {0x0112, 2, 71},
    {0x0116, 4, 75}, {0x0122, 2, 76}, {0x012a, 2, 78}, {0x012e, 2, 65}, {0x0136, 2, 77}, {0x013b, 2, 79}, {0x0141, 6, 89}, {0x014c, 2, 84},
    {0x0156, 2, 42}, {0x015a, 2, 90}, {0x0160, 2, 80}, {0x016a, 2, 91}, {0x0172, 2, 88}, {0x0179, 6, 74}, {0x2019, 1,127}, {0x201c, 3, 52},
};

static const MapEntry latin14Mapping[25] = {
    {0x0080,33,  0}, {0x00a3, 1, 35}, {0x00a7, 1, 39}, {0x00a9, 1, 41}, {0x00ad, 2, 45}, {0x00b6, 1, 54}, {0x00c0,16, 64}, {0x00d1, 6, 81},
    {0x00d8, 6, 88}, {0x00df,17, 95}, {0x00f1, 6,113}, {0x00f8, 6,120}, {0x00ff, 1,127}, {0x010a, 2, 36}, {0x0120, 2, 50}, {0x0174, 5, 80},
    {0x1e02, 2, 33}, {0x1e0a, 2, 38}, {0x1e1e, 2, 48}, {0x1e40, 2, 52}, {0x1e56, 2, 55}, {0x1e60, 2, 59}, {0x1e6a, 2, 87}, {0x1e80, 6, 40},
    {0x1ef2, 2, 44},
};

static const MapEntry latin15Mapping[12] = {
    {0x0080,36,  0}, {0x00a5, 1, 37}, {0x00a7, 1, 39}, {0x00a9,11, 41}, {0x00b5, 3, 53}, {0x00b9, 3, 57}, {0x00bf,65, 63}, {0x0152, 2, 60},
    {0x0160, 2, 38}, {0x0178, 1, 62}, {0x017d, 2, 52}, {0x20ac, 1, 36},
};

static const MapEntry latin16Mapping[34] = {
    {0x0080,33,  0}, {0x00a7, 1, 39}, {0x00a9, 1, 41}, {0x00ab, 1, 43}, {0x00ad, 1, 45}, {0x00b0, 2, 48}, {0x00b6, 2, 54}, {0x00bb, 1, 59},
    {0x00c0, 3, 64}, {0x00c4, 1, 68}, {0x00c6,10, 70}, {0x00d2, 3, 82}, {0x00d6, 1, 86}, {0x00d9, 4, 89}, {0x00df, 4, 95}, {0x00e4, 1,100},
    {0x00e6,10,102}, {0x00f2, 3,114}, {0x00f6, 1,118}, {0x00f9, 4,121}, {0x00ff, 1,127}, {0x0102, 6, 67}, {0x010c, 2, 50}, {0x0110, 2, 80},
    {0x0118, 2, 93}, {0x0141, 4, 35}, {0x0150, 4, 85}, {0x015a, 2, 87}, {0x0160, 2, 38}, {0x0170, 2, 88}, {0x0178, 7, 62}, {0x0218, 4, 42},
    {0x201d, 2, 53}, {0x20ac, 1, 36},
};

static const MapEntry cp437Mapping[58] = {
    {0x00a0, 4,127}, {0x00a5, 1, 29}, {0x00aa, 3, 38}, {0x00b0, 3,120}, {0x00b5, 1,102}, {0x00b7, 1,122}, {0x00ba, 4, 39}, {0x00bf, 1, 40},
    {0x00c4, 4, 14}, {0x00c9, 1, 16}, {0x00d1, 1, 37}, {0x00d6, 1, 25}, {0x00dc, 1, 26}, {0x00df, 4, 97}, {0x00e4,12,  4}, {0x00f1, 4, 36},
    {0x00f6, 2, 20}, {0x00f9, 4, 23}, {0x00ff, 1, 24}, {0x0192, 1, 31}, {0x0393, 1, 98}, {0x0398, 1,105}, {0x03a3, 1,100}, {0x03a6, 1,104},
    {0x03a9, 1,106}, {0x03b1, 1, 96}, {0x03b4, 2,107}, {0x03c0, 1, 99}, {0x03c3, 2,101}, {0x03c6, 1,109}, {0x207f, 1,124}, {0x20a7, 1, 30},
    {0x2219, 2,121}, {0x221e, 1,108}, {0x2229, 1,111}, {0x2248, 1,119}, {0x2261, 1,112}, {0x2264, 2,115}, {0x2310, 1, 41}, {0x2320, 2,116},
    {0x2500, 1, 68}, {0x2502, 1, 51}, {0x250c, 1, 90}, {0x2510, 1, 63}, {0x2514, 1, 64}, {0x2518, 1, 89}, {0x251c, 1, 67}, {0x2524, 1, 52},
    {0x252c, 1, 66}, {0x2534, 1, 65}, {0x253c, 1, 69}, {0x2550,29, 77}, {0x2580, 1, 95}, {0x2584, 1, 92}, {0x2588, 1, 91}, {0x258c, 1, 93},
    {0x2590, 4, 94}, {0x25a0, 1,126},
};

static const MapEntry cp1250Mapping[55] = {
    {0x00a0, 1, 32}, {0x00a4, 1, 36}, {0x00a6, 4, 38}, {0x00ab, 4, 43}, {0x00b0, 2, 48}, {0x00b4, 5, 52}, {0x00bb, 1, 59}, {0x00c1, 2, 65},
    {0x00c4, 1, 68}, {0x00c7, 1, 71}, {0x00c9, 1, 73}, {0x00cb, 1, 75}, {0x00cd, 2, 77}, {0x00d3, 2, 83}, {0x00d6, 2, 86}, {0x00da, 1, 90},
    {0x00dc, 2, 92}, {0x00df, 1, 95}, {0x00e1, 2, 97}, {0x00e4, 1,100}, {0x00e7, 1,103}, {0x00e9, 1,105}, {0x00eb, 1,107}, {0x00ed, 2,109},
    {0x00f3, 2,115}, {0x00f6, 2,118}, {0x00fa, 1,122}, {0x00fc, 2,124}, {0x0102, 6, 67}, {0x010c, 6, 72}, {0x0118, 4, 74}, {0x0139, 2, 69},
    {0x013d, 2, 60}, {0x0141, 4, 35}, {0x0147, 2, 82}, {0x0150, 2, 85}, {0x0154, 2, 64}, {0x0158, 4, 88}, {0x015e, 8, 42}, {0x016e, 4, 89},
    {0x0179, 6, 15}, {0x02c7, 1, 33}, {0x02d8, 2, 34}, {0x02db, 1, 50}, {0x02dd, 1, 61}, {0x2013, 2, 22}, {0x2018, 3, 17}, {0x201c, 3, 19},
    {0x2020, 3,  6}, {0x2026, 1,  5}, {0x2030, 1,  9}, {0x2039, 2, 11}, {0x20ac, 1,  0}, {0x2122, 1, 25}, {0xfffd, 1, 24},
};

static const MapEntry cp1251Mapping[24] = {
    {0x00a0, 1, 32}, {0x00a4, 1, 36}, {0x00a6, 2, 38}, {0x00a9, 1, 41}, {0x00ab, 4, 43}, {0x00b0, 2, 48}, {0x00b5, 3, 53}, {0x00bb, 1, 59},
    {0x0401,12, 40}, {0x040e,66, 33}, {0x0451,12, 56}, {0x045e, 2, 34}, {0x0490, 2, 37}, {0x2013, 2, 22}, {0x2018, 3, 17}, {0x201c, 3, 19},
    {0x2020, 3,  6}, {0x2026, 1,  5}, {0x2030, 1,  9}, {0x2039, 2, 11}, {0x20ac, 1,  8}, {0x2116, 1, 57}, {0x2122, 1, 25}, {0xfffd, 1, 24},
};

static const MapEntry cp1252Mapping[18] = {
    {0x00a0,96, 32}, {0x0152, 2, 12}, {0x0160, 2, 10}, {0x0178, 1, 31}, {0x017d, 2, 14}, {0x0192, 1,  3}, {0x02c6, 1,  8}, {0x02dc, 1, 24},
    {0x2013, 2, 22}, {0x2018, 3, 17}, {0x201c, 3, 19}, {0x2020, 3,  6}, {0x2026, 1,  5}, {0x2030, 1,  9}, {0x2039, 2, 11}, {0x20ac, 1,  0},
    {0x2122, 1, 25}, {0xfffd, 1, 29},
};

static const MapEntry cp1256Mapping[43] = {
    {0x00a0, 1, 32}, {0x00a2, 8, 34}, {0x00ab,15, 43}, {0x00bb, 4, 59}, {0x00d7, 1, 87}, {0x00e0, 1, 96}, {0x00e2, 1, 98}, {0x00e7, 5,103},
    {0x00ee, 2,110}, {0x00f4, 1,116}, {0x00f7, 1,119}, {0x00f9, 1,121}, {0x00fb, 2,123}, {0x0152, 2, 12}, {0x0192, 1,  3}, {0x02c6, 1,  8},
    {0x060c, 1, 33}, {0x061b, 1, 58}, {0x061f, 1, 63}, {0x0621,26, 65}, {0x0640,19, 92}, {0x0679, 1, 10}, {0x067e, 1,  1}, {0x0686, 1, 13},
    {0x0688, 1, 15}, {0x0691, 1, 26}, {0x0698, 1, 14}, {0x06a9, 1, 24}, {0x06af, 1, 16}, {0x06ba, 1, 31}, {0x06be, 1, 42}, {0x06c1, 1, 64},
    {0x06d2, 1,127}, {0x200c, 4, 29}, {0x2013, 2, 22}, {0x2018, 3, 17}, {0x201c, 3, 19}, {0x2020, 3,  6}, {0x2026, 1,  5}, {0x2030, 1,  9},
    {0x2039, 2, 11}, {0x20ac, 1,  0}, {0x2122, 1, 25},
};


static uint8_t unicodeToCharcode(uint16_t unicode, const MapEntry* entries, size_t entryCount)
{
	auto it = std::upper_bound(entries, entries + entryCount, unicode);
	if (it != entries)
	{
		--it;
		if (unicode < it->unicode + it->count)
		{
			return it->charcode + (unicode - it->unicode) + 128;
		}
	}
	throw std::invalid_argument("Unexpected charcode");
}

static void mapFromUnicode(const std::wstring& str, const MapEntry* entries, size_t entryCount, std::string& bytes)
{
	bytes.reserve(str.length());
	for (wchar_t c : str) {
		if (c < 0x80) {
			bytes.push_back(static_cast<char>(c));
		}
		else {
			bytes.push_back(static_cast<char>(unicodeToCharcode(c, entries, entryCount)));
		}
	}
}

#define CONVERT_USING(table, str, bytes) mapFromUnicode(str, table, Length(table), bytes)

} // anonymous

void
TextEncoder::GetBytes(const std::wstring& str, CharacterSet charset, std::string& bytes)
{
	bytes.clear();

	switch (charset)
	{
	case CharacterSet::Unknown:
	case CharacterSet::ISO8859_1:
		bytes.reserve(str.length());
		for (wchar_t c : str) {
			if (c < 0xff) {
				bytes.push_back(static_cast<char>(c));
			}
			else {
				throw std::invalid_argument("Unexpected charcode");
			}
		}
		break;
	case CharacterSet::ASCII:
	{
		bytes.reserve(str.length());
		for (wchar_t c : str) {
			if (c < 0x80) {
				bytes.push_back(static_cast<char>(c));
			}
			else {
				throw std::invalid_argument("Unexpected charcode");
			}
		}
		break;
	}
	case CharacterSet::ISO8859_2:
		CONVERT_USING(latin2Mapping, str, bytes); break;
	case CharacterSet::ISO8859_3:
		CONVERT_USING(latin3Mapping, str, bytes); break;
	case CharacterSet::ISO8859_4:
		CONVERT_USING(latin4Mapping, str, bytes); break;
	case CharacterSet::ISO8859_5:
		CONVERT_USING(latin5Mapping, str, bytes); break;
	case CharacterSet::ISO8859_6:
		CONVERT_USING(latin6Mapping, str, bytes); break;
	case CharacterSet::ISO8859_7:
		CONVERT_USING(latin7Mapping, str, bytes); break;
	case CharacterSet::ISO8859_8:
		CONVERT_USING(latin8Mapping, str, bytes); break;
	case CharacterSet::ISO8859_9:
		CONVERT_USING(latin9Mapping, str, bytes); break;
	case CharacterSet::ISO8859_10:
		CONVERT_USING(latin10Mapping, str, bytes); break;
	case CharacterSet::ISO8859_11:
		CONVERT_USING(latin11Mapping, str, bytes); break;
	case CharacterSet::ISO8859_13:
		CONVERT_USING(latin13Mapping, str, bytes); break;
	case CharacterSet::ISO8859_14:
		CONVERT_USING(latin14Mapping, str, bytes); break;
	case CharacterSet::ISO8859_15:
		CONVERT_USING(latin15Mapping, str, bytes); break;
	case CharacterSet::ISO8859_16:
		CONVERT_USING(latin16Mapping, str, bytes); break;
	case CharacterSet::Cp437:
		CONVERT_USING(cp437Mapping, str, bytes); break;
	case CharacterSet::Cp1250:
		CONVERT_USING(cp1250Mapping, str, bytes); break;
	case CharacterSet::Cp1251:
		CONVERT_USING(cp1251Mapping, str, bytes); break;
	case CharacterSet::Cp1252:
		CONVERT_USING(cp1252Mapping, str, bytes); break;
	case CharacterSet::Cp1256:
		CONVERT_USING(cp1256Mapping, str, bytes); break;
	case CharacterSet::Shift_JIS:
		JPTextEncoder::EncodeShiftJIS(str, bytes); break;
	case CharacterSet::Big5:
		Big5TextEncoder::EncodeBig5(str, bytes); break;
	case CharacterSet::GB2312:
		GBTextEncoder::EncodeGB2312(str, bytes); break;
	case CharacterSet::GB18030:
		GBTextEncoder::EncodeGB18030(str, bytes); break;
	case CharacterSet::EUC_JP:
		JPTextEncoder::EncodeEUCJP(str, bytes); break;
	case CharacterSet::EUC_KR:
		KRTextDecoder::EncodeEucKr(str, bytes); break;
	case CharacterSet::UTF8:
		TextUtfEncoding::ToUtf8(str, bytes); break;
	default:
		break;
	}
}

} // ZXing
