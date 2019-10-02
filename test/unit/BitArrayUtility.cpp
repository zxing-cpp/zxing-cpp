/*
* Copyright 2017 Huy Cuong Nguyen
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
#include "BitArrayUtility.h"
#include "BitArray.h"

namespace ZXing { namespace Utility {

std::string ToString(const BitArray& arr)
{
	return ToString(arr, 'X', '.');
}

std::string ToString(const BitArray& arr, char one, char zero)
{
	std::string result;
	result.reserve(arr.size());
	auto iter = arr.begin();
	for (int i = 0; i < arr.size(); ++i, ++iter) {
		result.push_back(*iter != 0 ? one : zero);
	}
	return result;
}

BitArray ParseBitArray(const std::string& str)
{
	return ParseBitArray(str, 'X');
}

BitArray ParseBitArray(const std::string& str, char one)
{
	BitArray result((int)str.length());
	auto iter = result.begin();
	for (size_t x = 0; x < str.length(); ++x, ++iter) {
		if (str[x] == one) {
			result.set(static_cast<int>(x));
		}
	}
	return result;
}

}} // ZXing::Utility
