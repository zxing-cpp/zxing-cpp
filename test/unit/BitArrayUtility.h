#pragma once

#include <string>
#include <string_view>

namespace ZXing {

class BitArray;

namespace Utility {

	std::string ToString(const BitArray& arr, char one, char zero);
	std::string ToString(const BitArray& BitArray);
	BitArray ParseBitArray(std::string_view str, char one = 'X');

}} // ZXing::Utility
