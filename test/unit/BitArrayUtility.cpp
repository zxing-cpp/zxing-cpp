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
	for (bool bit : arr)
		result.push_back(bit ? one : zero);

	return result;
}

BitArray ParseBitArray(std::string_view str, char one)
{
	BitArray result(Size(str));
	for (int i = 0; i < Size(str); ++i)
		result.set(i, str[i] == one);
	return result;
}

}} // ZXing::Utility
