/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace ZXing {

/**
	ByteArray is an extension of std::vector<unsigned char>.
*/
class ByteArray : public std::vector<uint8_t>
{
public:
	ByteArray() = default;
	ByteArray(std::initializer_list<uint8_t> list) : std::vector<uint8_t>(list) {}
	explicit ByteArray(int len) : std::vector<uint8_t>(len, 0) {}
	explicit ByteArray(const std::string& str) : std::vector<uint8_t>(str.begin(), str.end()) {}

	void append(const ByteArray& other) { insert(end(), other.begin(), other.end()); }
};

inline std::string ToHex(const ByteArray& bytes)
{
	std::string res(bytes.size() * 3, ' ');

	for (size_t i = 0; i < bytes.size(); ++i)
		sprintf(&res[i * 3], "%02X ", bytes[i]);

	return res.substr(0, res.size()-1);
}

} // ZXing
