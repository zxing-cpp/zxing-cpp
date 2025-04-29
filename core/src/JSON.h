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

inline std::string JsonKeyValue(std::string_view key, std::string_view val)
{
	return val.empty() ? std::string() : StrCat("\"", key, "\":", val, ',');
}

template<typename T>
inline std::string JsonValue(std::string_view key, T val)
{
	if constexpr (std::is_same_v<T, bool>)
		return val ? JsonKeyValue(key, "true") : "";
	else if constexpr (std::is_arithmetic_v<T>)
		return JsonKeyValue(key, std::to_string(val));
	else if constexpr (std::is_convertible_v<T, std::string_view>)
		return JsonKeyValue(key, StrCat("\"" , val, "\""));
	else
		static_assert("unsupported JSON value type");
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
	else if constexpr (std::is_same_v<std::string_view, T>)
		return str;
	else
		static_assert("unsupported JSON value type");
}

} // ZXing
