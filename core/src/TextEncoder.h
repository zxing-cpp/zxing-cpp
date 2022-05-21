/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

enum class CharacterSet;

class TextEncoder
{
	static void GetBytes(const std::wstring& str, CharacterSet charset, std::string& bytes);
public:
	static std::string FromUnicode(const std::wstring& str, CharacterSet charset) {
		std::string r;
		GetBytes(str, charset, r);
		return r;
	}
};

} // ZXing
