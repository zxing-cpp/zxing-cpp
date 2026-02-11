#include "ZXing/MultiFormatWriter.h"
#include "ZXing/BitMatrix.h"
#include "ZXing/BitMatrixIO.h"

#include <iostream>

int main(int argc, char** argv)
{
	using namespace ZXing;

	MultiFormatWriter writer(BarcodeFormat::QRCode);
	BitMatrix bitmatrix = writer.encode("Hello, World!", 200, 200);
	auto matrix = ToMatrix<int8_t>(bitmatrix);
	std::cout << ToString(bitmatrix) << "\n";

	return 0;
}
