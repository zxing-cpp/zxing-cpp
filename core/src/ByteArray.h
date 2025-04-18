/*
* Copyright 2016 Nu-book Inc.
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Range.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace ZXing {

/**
	ByteArray is an extension of std::vector<uint8_t>.
*/
class ByteArray : public std::vector<uint8_t>
{
public:
	ByteArray() = default;
	ByteArray(std::initializer_list<uint8_t> list) : std::vector<uint8_t>(list) {}
	explicit ByteArray(int len) : std::vector<uint8_t>(len, 0) {}
	explicit ByteArray(const std::string& str) : std::vector<uint8_t>(str.begin(), str.end()) {}

	void append(ByteView other) { insert(end(), other.begin(), other.end()); }
	void append(std::string_view other) { insert(end(), other.begin(), other.end()); }

	std::string_view asString(size_t pos = 0, size_t len = std::string_view::npos) const
	{
		return std::string_view(reinterpret_cast<const char*>(data()), size()).substr(pos, len);
	}

	ByteView asView(size_t pos = 0, size_t len = size_t(-1)) const
	{
		return ByteView(*this).subview(pos, len);
	}
};

inline std::string ToHex(ByteView bytes)
{
	std::string res(bytes.size() * 3, ' ');

	for (size_t i = 0; i < bytes.size(); ++i)
	{
#ifdef _MSC_VER
		sprintf_s(&res[i * 3], 4, "%02X ", bytes[i]);
#else
		snprintf(&res[i * 3], 4, "%02X ", bytes[i]);
#endif
	}

	return res.substr(0, res.size()-1);
}

} // ZXing
