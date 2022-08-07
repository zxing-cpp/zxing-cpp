/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <string_view>

namespace ZXing {

std::string ToUtf8(std::wstring_view str);
std::wstring FromUtf8(std::string_view utf8);

std::wstring EscapeNonGraphical(std::wstring_view str);
std::string EscapeNonGraphical(std::string_view utf8);

} // namespace ZXing
