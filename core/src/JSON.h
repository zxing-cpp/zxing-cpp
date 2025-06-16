/*
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ZXAlgorithms.h"

#include <cstring>
#include <optional>
#include <string>
#include <string_view>

namespace ZXing {

std::string JsonEscapeStr(std::string_view str);
std::string JsonUnEscapeStr(std::string_view str);

template<typename T>
inline std::string JsonProp(std::string_view key, T val, T ignore = {})
{
	if (val == ignore)
		return {};

	#define ZX_JSON_KEY_VAL(...) StrCat("\"", key, "\":", __VA_ARGS__, ',')
	if constexpr (std::is_same_v<T, bool>)
		return val ? ZX_JSON_KEY_VAL("true") : "";
	else if constexpr (std::is_arithmetic_v<T>)
		return ZX_JSON_KEY_VAL(std::to_string(val));
	else if constexpr (std::is_convertible_v<T, std::string_view>)
		return ZX_JSON_KEY_VAL("\"" , JsonEscapeStr(val), "\"");
	else
		static_assert("unsupported JSON value type");
	#undef ZX_JSON_KEY_VAL
}

std::string_view JsonGetStr(std::string_view json, std::string_view key);

template <typename T>
inline std::optional<T> JsonGet(std::string_view json, std::string_view key)
{
	auto str = JsonGetStr(json, key);
	if (!str.data())
		return std::nullopt;

	if constexpr (std::is_same_v<bool, T>)
		return str.empty() || Contains("1tT", str.front()) ? std::optional(true) : std::nullopt;
	else if constexpr (std::is_arithmetic_v<T>)
		return str.empty() ? std::nullopt : std::optional(FromString<T>(str));
	else if constexpr (std::is_same_v<std::string, T>)
		return JsonUnEscapeStr(str);
	else
		static_assert("unsupported JSON value type");
}

} // ZXing
