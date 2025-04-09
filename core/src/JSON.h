/*
* Copyright 2025 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstring>
#include <string>

#include "ZXAlgorithms.h"

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

inline bool JsonGetBool(std::string_view json, std::string_view key)
{
	auto posKey = json.find(key);
	if (posKey == std::string_view::npos || key.empty())
		return false;

	auto posSep = posKey + key.size();
	if (posSep == json.size() || json[posSep] == ',')
		return true;

	if (json[posSep] != ':')
		return false;

	return posSep < json.size() - 1 && Contains("1tT", json[posSep + 1]);
}

inline std::string_view JsonGetStr(std::string_view json, std::string_view key)
{
	auto posKey = json.find(key);
	if (posKey == std::string_view::npos)
		return {};

	auto posSep = posKey + key.size();
	if (posSep + 1 >= json.size() || json[posSep] != ':')
		return {};

	return json.substr(posSep + 1, json.find_first_of(',', posSep + 1) - posSep - 1);
}

template<typename T> T JsonGet(std::string_view json, std::string_view key)
{
	if constexpr (std::is_same_v<bool, T>)
		return JsonGetBool(json, key);
	if constexpr (std::is_same_v<std::string_view, T>)
		return JsonGetStr(json, key);

	throw UnsupportedError("internal error");
}

} // ZXing
