/*
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Error.h"

#include <charconv>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ZXing {

inline std::string JsonValue(std::string_view key, std::string_view val, int indent = 0)
{
	//TODO: use std::format from c++20
	return val.empty() ? std::string() : std::string(indent * 2, ' ') + "\"" + std::string(key) + "\":\"" + std::string(val) + "\",";
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline std::string JsonValue(std::string_view key, T val, int indent = 0)
{
	if constexpr (std::is_same_v<T, bool>)
		return JsonValue(key, val ? "true" : "false", indent);
	else
		return JsonValue(key, std::to_string(val), indent);
}

bool JsonGetBool(std::string_view json, std::string_view key);
std::string_view JsonGetStr(std::string_view json, std::string_view key);

template <typename T>
inline T JsonGet(std::string_view json, std::string_view key)
{
	if constexpr (std::is_same_v<bool, T>)
		return JsonGetBool(json, key);
	if constexpr (std::is_same_v<std::string_view, T>)
		return JsonGetStr(json, key);

	throw UnsupportedError("internal error");
}

inline int svtoi(std::string_view sv)
{
	int val = 0;
	auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
	if (ec != std::errc() || ptr != sv.data() + sv.size())
		throw std::invalid_argument("failed to parse int from '" + std::string(sv) + "'");

	return val;
}

} // ZXing
