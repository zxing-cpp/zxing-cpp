/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "CharacterSetECI.h"

#include "ECI.h"
#include "TextDecoder.h"

#include <cctype>
#include <iomanip>
#include <map>
#include <sstream>
#include <utility>

namespace ZXing::CharacterSetECI {

struct CompareNoCase {
	bool operator ()(const char* a, const char* b) const {
		while (*a != '\0' && *b != '\0') {
			auto ca = std::tolower(*a++);
			auto cb = std::tolower(*b++);
			if (ca < cb) {
				return true;
			}
			else if (ca > cb) {
				return false;
			}
		}
		return *a == '\0' && *b != '\0';
	}
};

static const std::map<const char *, CharacterSet, CompareNoCase> ECI_NAME_TO_CHARSET = {
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
	{"EUC-CN",		CharacterSet::GB18030},
	{"GBK",			CharacterSet::GB18030},
	{"EUC_KR",		CharacterSet::EUC_KR},
	{"EUC-KR",		CharacterSet::EUC_KR},
	{"BINARY",		CharacterSet::BINARY},
};

CharacterSet CharsetFromName(const char* name)
{
	auto it = ECI_NAME_TO_CHARSET.find(name);
	if (it != ECI_NAME_TO_CHARSET.end()) {
		return it->second;
	}
	return CharacterSet::Unknown;
}

} // namespace ZXing::CharacterSetECI
