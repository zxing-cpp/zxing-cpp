/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BarcodeFormat.h"
#include "CreateBarcode.h"
#include "ReadBarcode.h"
#include "WriteBarcode.h"
#include "Version.h"

namespace ZXing {

const std::string& Version();

} // namespace ZXing

/**
 \mainpage Introduction

ZXing-C++ ("zebra crossing") is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.

The central class in the library is ZXing::Barcode, which represents a decoded or created barcode symbol, providing access to its
content, format, position, and other metadata. It serves as the primary interface for working with barcodes in the library.

 \section reader_sec Reading barcodes

1. Load your image into memory (3rd-party library required).
2. Call ZXing::ReadBarcodes(), the simplest API to get a list of ZXing::Barcode objects.

A very simple example looks like this:
```c++
#include "ZXing/ZXingCpp.h"
#include <iostream>

int main(int argc, char** argv)
{
	int width, height;
	unsigned char* data;
	// load your image data from somewhere. ImageFormat::Lum assumes grey scale image data.

	auto image = ZXing::ImageView(data, width, height, ZXing::ImageFormat::Lum);
	auto options = ZXing::ReaderOptions().formats(ZXing::BarcodeFormat::QRCode);
	auto barcodes = ZXing::ReadBarcodes(image, options);

	for (const auto& b : barcodes)
		std::cout << ZXing::ToString(b.format()) << ": " << b.text() << "\n";

	return 0;
}
```

 \section writer_sec Writing barcodes

1. Create a ZXing::Barcode object using either ZXing::CreateBarcodeFromText() or ZXing::CreateBarcodeFromBytes().
2. Call ZXing::WriteBarcodeToImage() or ZXing::WriteBarcodeToSVG() to get the output.

A very simple example looks like this:
```c++
#include "ZXing/ZXingCpp.h"
#include <iostream>

int main()
{
	auto barcode = ZXing::CreateBarcodeFromText("Hello, World!", ZXing::BarcodeFormat::QRCode);
	auto options = ZXing::WriterOptions().scale(4).addQuietZones(false);
	auto svg = ZXing::WriteBarcodeToSVG(barcode, options);

    std::cout << svg << std::endl;

	return 0;
}
```
*/
