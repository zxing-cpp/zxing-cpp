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
			result.set(x);
		}
	}
	return result;
}

}} // ZXing::Utility
