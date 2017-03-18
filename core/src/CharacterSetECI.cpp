/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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

#include "CharacterSetECI.h"
#include "CharacterSet.h"

#include <map>
#include <cstring>

namespace ZXing {

namespace {

static const std::map<int, CharacterSet> ECI_VALUE_TO_CHARSET = {
	{0,  CharacterSet::Cp437},
	{1,  CharacterSet::ISO8859_1},
	{2,  CharacterSet::Cp437},
	{3,  CharacterSet::ISO8859_1},
	{4,  CharacterSet::ISO8859_2},
	{5,  CharacterSet::ISO8859_3},
	{6,  CharacterSet::ISO8859_4},
	{7,  CharacterSet::ISO8859_5},
	{8,  CharacterSet::ISO8859_6},
	{9,  CharacterSet::ISO8859_7},
	{10, CharacterSet::ISO8859_8},
	{11, CharacterSet::ISO8859_9},
	{12, CharacterSet::ISO8859_10},
	{13, CharacterSet::ISO8859_11},
	{15, CharacterSet::ISO8859_13},
	{16, CharacterSet::ISO8859_14},
	{17, CharacterSet::ISO8859_15},
	{18, CharacterSet::ISO8859_16},
	{20, CharacterSet::Shift_JIS},
	{21, CharacterSet::Cp1250},
	{22, CharacterSet::Cp1251},
	{23, CharacterSet::Cp1252},
	{24, CharacterSet::Cp1256},
	{25, CharacterSet::UnicodeBig},
	{26, CharacterSet::UTF8},
	{27, CharacterSet::ASCII},
	{28, CharacterSet::Big5},
	{29, CharacterSet::GB18030},
	{30, CharacterSet::EUC_KR},
	{170, CharacterSet::ASCII},
};

struct StringCmp {
	bool operator()(char const *a, char const *b) const {
		return std::strcmp(a, b) < 0;
	}
};

static const std::map<const char*, CharacterSet, StringCmp> ECI_NAME_TO_CHARSET = {
	{"Cp437",		CharacterSet::Cp437},
	{"ISO8859_1",	CharacterSet::ISO8859_1},
	{"ISO-8859-1",	CharacterSet::ISO8859_1},
	{"ISO8859_2",	CharacterSet::ISO8859_2},
	{"ISO-8859-2",	CharacterSet::ISO8859_2},
	{"ISO8859_3",	CharacterSet::ISO8859_3},
	{"ISO-8859-3",	CharacterSet::ISO8859_3},
	{"ISO8859_4",	CharacterSet::ISO8859_4},
	{"ISO-8859-4",	CharacterSet::ISO8859_4},
	{"ISO8859_5",	CharacterSet::ISO8859_5},
	{"ISO-8859-5",	CharacterSet::ISO8859_5},
	{"ISO8859_6",	CharacterSet::ISO8859_6},
	{"ISO-8859-6",	CharacterSet::ISO8859_6},
	{"ISO8859_7",	CharacterSet::ISO8859_7},
	{"ISO-8859-7",	CharacterSet::ISO8859_7},
	{"ISO8859_8",	CharacterSet::ISO8859_8},
	{"ISO-8859-8",	CharacterSet::ISO8859_8},
	{"ISO8859_9",	CharacterSet::ISO8859_9},
	{"ISO-8859-9",	CharacterSet::ISO8859_9},
	{"ISO8859_10",	CharacterSet::ISO8859_10},
	{"ISO-8859-10",	CharacterSet::ISO8859_10},
	{"ISO8859_11",	CharacterSet::ISO8859_11},
	{"ISO-8859-11",	CharacterSet::ISO8859_11},
	{"ISO8859_13",	CharacterSet::ISO8859_13},
	{"ISO-8859-13",	CharacterSet::ISO8859_13},
	{"ISO8859_14",	CharacterSet::ISO8859_14},
	{"ISO-8859-14",	CharacterSet::ISO8859_14},
	{"ISO8859_15",	CharacterSet::ISO8859_15},
	{"ISO-8859-15",	CharacterSet::ISO8859_15},
	{"ISO8859_16",	CharacterSet::ISO8859_16},
	{"ISO-8859-16",	CharacterSet::ISO8859_16},
	{"SJIS",		CharacterSet::Shift_JIS},
	{"Shift_JIS",	CharacterSet::Shift_JIS},
	{"Cp1250",		CharacterSet::Cp1250},
	{"windows-1250",CharacterSet::Cp1250},
	{"Cp1251",		CharacterSet::Cp1251},
	{"windows-1251",CharacterSet::Cp1251},
	{"Cp1252",		CharacterSet::Cp1252},
	{"windows-1252",CharacterSet::Cp1252},
	{"Cp1256",		CharacterSet::Cp1256},
	{"windows-1256",CharacterSet::Cp1256},
	{"UnicodeBigUnmarked", CharacterSet::UnicodeBig},
	{"UTF-16BE",	CharacterSet::UnicodeBig},
	{"UnicodeBig",	CharacterSet::UnicodeBig},
	{"UTF8",		CharacterSet::UTF8},
	{"UTF-8",		CharacterSet::UTF8},
	{"ASCII",		CharacterSet::ASCII},
	{"US-ASCII",	CharacterSet::ASCII},
	{"Big5",		CharacterSet::Big5},
	{"GB2312",		CharacterSet::GB2312},
	{"GB18030",		CharacterSet::GB18030},
	{"EUC_CN",		CharacterSet::GB18030},
	{"EUC-CN",		CharacterSet::GB18030 },
	{"GBK",			CharacterSet::GB18030},
	{"EUC_KR",		CharacterSet::EUC_KR},
	{"EUC-KR",		CharacterSet::EUC_KR },
};

} // anonymous

CharacterSet
CharacterSetECI::CharsetFromValue(int value)
{
	auto it = ECI_VALUE_TO_CHARSET.find(value);
	if (it != ECI_VALUE_TO_CHARSET.end())
	{
		return it->second;
	}
	return CharacterSet::Unknown;
}

int
CharacterSetECI::ValueForCharset(CharacterSet charset)
{
	for (auto& entry : ECI_VALUE_TO_CHARSET)
	{
		if (entry.second == charset)
		{
			return entry.first;
		}
	}
	return 0;
}

CharacterSet
CharacterSetECI::CharsetFromName(const char* name)
{
	auto it = ECI_NAME_TO_CHARSET.find(name);
	if (it != ECI_NAME_TO_CHARSET.end())
	{
		return it->second;
	}
	return CharacterSet::Unknown;
};

} // ZXing
