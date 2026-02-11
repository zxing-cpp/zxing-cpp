#include "ZXing/MultiFormatWriter.h"
#include "ZXing/BitMatrix.h"
#include "ZXing/BitMatrixIO.h"
#include "ZXing/ZXVersion.h"

#include <iostream>

int main(int argc, char** argv)
{
	using namespace ZXing;

	std::cout << "zxing-cpp version " << ZXING_VERSION_STR << "\n";

	MultiFormatWriter writer(BarcodeFormat::QRCode);
	BitMatrix bitmatrix = writer.encode("Hello, World!", 200, 200);
	auto matrix = ToMatrix<int8_t>(bitmatrix);
	std::cout << ToString(bitmatrix) << "\n";

	return 0;
}
