/*
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "JSON.h"

#include <algorithm>
#include <string_view>

// This code is trying to find the value of a key-value pair in a string of those.
// The input could be valid JSON, like '{"key": "val"}' or a stipped down version like
// 'key:val'. This is also compatible with the string serialization of a python dictionary.
// This could easily be done with the following regex (see below).
// But using std::regex adds 140k+ to the binary size. ctre is _very_ nice to use and has
// an even smaller binary footpint than the hand-roled code below! But I don't want to add
// 5k lines of c++ code for that.

// #define ZXING_USE_CTRE
#ifdef ZXING_USE_CTRE
#pragma GCC diagnostic ignored "-Wundef" // see https://github.com/hanickadot/compile-time-regular-expressions/issues/340
#include "ctre.hpp"
#pragma GCC diagnostic error "-Wundef"

static constexpr auto PATTERN = ctll::fixed_string{R"(\"?([[:alpha:]][[:alnum:]]*)\"?\s*(?:[:]\s*\"?([[:alnum:]]+)\"?)?(?:,|$))"};
#else
#include "ZXAlgorithms.h"
#endif

namespace ZXing {

inline bool CmpCI(unsigned char ch1, unsigned char ch2)
{
	return std::tolower(ch1) == std::tolower(ch2);
}

inline size_t FindCaseInsensitive(std::string_view haystack, std::string_view needle)
{
	if (haystack.empty() || needle.empty())
		return std::string_view::npos;

	auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), CmpCI);

	return it != haystack.end() ? it - haystack.begin() : std::string_view::npos;
}

inline bool IsEqualCaseInsensitive(std::string_view str1, std::string_view str2)
{
	return str1.size() == str2.size() && std::equal(str1.begin(), str1.end(), str2.begin(), CmpCI);
}

std::string_view JsonGetStr(std::string_view json, std::string_view key)
{
#ifdef ZXING_USE_CTRE
	for (auto [ma, mk, mv] : ctre::search_all<PATTERN>(json))
		if (IsEqualCaseInsensitive(key, mk))
			return mv;

	return {};
#else
	auto &npos =  std::string_view::npos;
	auto posKey = FindCaseInsensitive(json, key);
	if (posKey == npos || (posKey > 0 && !Contains("\", \t\n\r", json[posKey - 1])))
		return {};

	auto posVal = json.find_first_not_of("\" \t", posKey + key.size());

	if (posVal == npos || json[posVal] == ',')
		return {json.data() + posKey, 0}; // return non-null data to signal that we found the key

	if (posVal >= json.size() - 1 || json[posVal++] != ':')
		return {};

	posVal = json.find_first_not_of("\" \t\n\r", posVal);
	auto posEnd = json.find_first_of(",\" \t\n\r", posVal);
	if (posEnd == npos)
		posEnd = json.size();

	return json.substr(posVal, posEnd - posVal);
#endif
}

bool JsonGetBool(std::string_view json, std::string_view key)
{
#ifdef ZXING_USE_CTRE
	for (auto [ma, mk, mv] : ctre::search_all<PATTERN>(json))
		if (IsEqualCaseInsensitive(key, mk))
			return mv.size() == 0 || Contains("1tT", *mv.data());

	return false;
#else
	auto val = JsonGetStr(json, key);

	return val.data() && (val.size() == 0 || Contains("1tT", val.front()));
#endif
}

} // ZXing
