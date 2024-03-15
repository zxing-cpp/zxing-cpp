/*
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"
#ifdef ZXING_EXPERIMENTAL_API
#include "WriteBarcode.h"
#else
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#endif

#include <QDebug>
#include <QImage>

namespace ZXingQt {

QImage WriteBarcode(QStringView text, ZXing::BarcodeFormat format)
{
	using namespace ZXing;

#ifdef ZXING_EXPERIMENTAL_API
	auto barcode = CreateBarcodeFromText(text.toString().toStdString(), format);
	auto bitmap = WriteBarcodeToImage(barcode);
#else
	auto writer = MultiFormatWriter(format);
	auto matrix = writer.encode(text.toString().toStdString(), 0, 0);
	auto bitmap = ToMatrix<uint8_t>(matrix);
#endif
	return QImage(bitmap.data(), bitmap.width(), bitmap.height(), bitmap.width(), QImage::Format::Format_Grayscale8).copy();
}

} // namespace ZXingQt

int main(int argc, char* argv[])
{
	if (argc != 4) {
		qDebug() << "usage: ZXingQtWriter <format> <text> <filename>";
		return 1;
	}

	auto format = ZXing::BarcodeFormatFromString(argv[1]);
	auto text = QString(argv[2]);
	auto filename = QString(argv[3]);

	auto result = ZXingQt::WriteBarcode(text, format);

	result.save(filename);

	return 0;
}
