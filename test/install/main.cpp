#include "ZXing/ZXingCpp.h"

#include <iostream>

int main(int argc, char** argv)
{
	using namespace ZXing;

	auto copts = CreatorOptions(BarcodeFormat::QRCode, "eclevel=L");
	auto barcode = CreateBarcodeFromText("Test", copts);
	auto wopts = WriterOptions().scale(2);
	auto img = WriteBarcodeToImage(barcode, wopts);

	auto iv = ImageView(img.data(), img.width(), img.height(), img.format());
	auto ropts = ReaderOptions().formats(BarcodeFormat::MatrixCodes);
	auto barcodes = ReadBarcodes(iv, ropts);

	for (const auto& b : barcodes)
		std::cout << ToString(b.format()) << ": " << b.text() << "\n";

	return 0;
}
