#pragma once
#include <string>

namespace ZXing {

class ByteMatrix;

namespace Utility {
    
	std::string ToString(const ByteMatrix& matrix, char one, char zero, char other, bool addSpace);
	std::string ToString(const ByteMatrix& matrix);

}} // ZXing::Utility
