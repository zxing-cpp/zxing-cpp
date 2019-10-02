#pragma once
#include <string>

namespace ZXing {

class BitArray;

namespace Utility {
    
	std::string ToString(const BitArray& arr, char one, char zero);
	std::string ToString(const BitArray& BitArray);
	BitArray ParseBitArray(const std::string& str, char one);
	BitArray ParseBitArray(const std::string& str);

}} // ZXing::Utility
