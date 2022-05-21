/*
* Copyright 2017 Huy Cuong Nguyen
*/
// SPDX-License-Identifier: Apache-2.0

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
