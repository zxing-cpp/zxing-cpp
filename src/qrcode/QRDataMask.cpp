#include "qrcode/QRDataMask.h"
#include "BitMatrix.h"

#include <stdexcept>

namespace ZXing {
namespace QRCode {

namespace {

/**
* 000: mask bits for which (x + y) mod 2 == 0
*/
bool DataMask000(int i, int j)
{
	return ((i + j) & 0x01) == 0;
}

/**
* 001: mask bits for which x mod 2 == 0
*/
bool DataMask001(int i, int j)
{
	return (i & 0x01) == 0;
}

/**
* 010: mask bits for which y mod 3 == 0
*/
bool DataMask010(int i, int j)
{
	return j % 3 == 0;
}

/**
* 011: mask bits for which (x + y) mod 3 == 0
*/
bool DataMask011(int i, int j)
{
	return (i + j) % 3 == 0;
}

/**
* 100: mask bits for which (x/2 + y/3) mod 2 == 0
*/
bool DataMask100(int i, int j)
{
	return (((i / 2) + (j / 3)) & 0x01) == 0;
}

/**
* 101: mask bits for which xy mod 2 + xy mod 3 == 0
*/
bool DataMask101(int i, int j)
{
	int temp = i * j;
	return (temp & 0x01) + (temp % 3) == 0;
}

/**
* 110: mask bits for which (xy mod 2 + xy mod 3) mod 2 == 0
*/
bool DataMask110(int i, int j)
{
	int temp = i * j;
	return (((temp & 0x01) + (temp % 3)) & 0x01) == 0;
}

/**
* 111: mask bits for which ((x+y)mod 2 + xy mod 3) mod 2 == 0
*/
bool DataMask111(int i, int j)
{
	return ((((i + j) & 0x01) + ((i * j) % 3)) & 0x01) == 0;
}

typedef bool(*IsMaskedFunc)(int, int);

/**
* See ISO 18004:2006 6.8.1
*/
static const IsMaskedFunc DATA_MASKS[] = {
	DataMask000,
	DataMask001,
	DataMask010,
	DataMask011,
	DataMask100,
	DataMask101,
	DataMask110,
	DataMask111,
};

} // anonymous

DataMask::DataMask(int reference)
{
	if (reference < 0 || reference > 7) {
		throw std::invalid_argument("Invalid data mask");
	}
	_isMasked = DATA_MASKS[reference];
}

void
DataMask::unmaskBitMatrix(BitMatrix& bits, int dimension) const
{
	for (int i = 0; i < dimension; i++) {
		for (int j = 0; j < dimension; j++) {
			if (_isMasked(i, j)) {
				bits.flip(j, i);
			}
		}
	}
}

} // QRCode
} // ZXing
