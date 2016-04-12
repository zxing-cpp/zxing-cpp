#pragma once
#include <vector>
#include <cstdint>

namespace ZXing {

/// <summary>
/// ByteArray is an extension of std::vector<unsigned char>.
/// </summary>
class ByteArray : public std::vector<uint8_t>
{
public:
	ByteArray()													{}
	explicit ByteArray(int len) : std::vector<uint8_t>(len, 0)	{}
	int length() const											{ return static_cast<int>(size()); }
	uint8_t* data()												{ return &(operator[](0)); }
	const uint8_t* data() const									{ return &(operator[](0)); }
	const char* charPtr() const									{ return (const char*)&(operator[](0)); }
	char* charPtr()												{ return (char*)&(operator[](0)); }
};

} // ZXing