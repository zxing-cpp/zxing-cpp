#pragma once
#include <iosfwd>
#include <string>

namespace ZXing {

class BitMatrix;

namespace Utility {
    
    void WriteBitMatrixAsPBM(const BitMatrix& matrix, std::ostream& out);
	std::string ToString(const BitMatrix& matrix, char one, char zero, bool addSpace);
	std::string ToString(const BitMatrix& matrix);
	BitMatrix ParseBitMatrix(const std::string& str, char one, bool expectSpace);
	BitMatrix ParseBitMatrix(const std::string& str);

}} // ZXing::Utility
