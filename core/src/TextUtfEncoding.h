/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <string_view>

namespace ZXing::TextUtfEncoding {

std::string ToUtf8(std::wstring_view str);
[[deprecated]] std::string ToUtf8(std::wstring_view str, const bool angleEscape);
std::wstring FromUtf8(std::string_view utf8);

std::wstring EscapeNonGraphical(std::wstring_view str);
std::string EscapeNonGraphical(std::string_view utf8);

} // namespace ZXing::TextUtfEncoding
