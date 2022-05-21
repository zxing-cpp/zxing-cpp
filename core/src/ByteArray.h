#pragma once
/*
* Copyright 2016 Nu-book Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

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
