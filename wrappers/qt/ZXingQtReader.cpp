/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingQt.h"

#include <QDebug>

using namespace ZXingQt;

void printBarcodes(const QList<Barcode>& barcodes)
{
	for (const auto& barcode : barcodes) {
		qDebug() << "Text:   " << barcode.text();
		qDebug() << "Format: " << barcode.format();
		qDebug() << "Content:" << barcode.contentType();
		qDebug() << "";
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		qDebug() << "Please supply exactly one image filename";
		return 1;
	}

	QString filePath = QString::fromUtf8(argv[1]);

	QImage image = QImage(filePath);

	if (image.isNull()) {
		qDebug() << "Could not load the filename as an image:" << filePath;
		return 1;
	}

#if 0 // simple free function use case
	auto options = ReaderOptions()
					 .setFormats(ZXing::BarcodeFormat::AllReadable)
					 .setTryInvert(false)
					 .setTextMode(ZXing::TextMode::HRI)
					 .setMaxNumberOfSymbols(10);

	auto barcodes = ReadBarcodes(image, options);

	printBarcodes(barcodes);

	return barcodes.isEmpty() ? 1 : 0;
#else // QObject with signal/slot use case
	BarcodeReader reader;
	reader.setFormats({BarcodeFormat::AllReadable});
	reader.setTryInvert(false);
	reader.setTextMode(TextMode::HRI);
	QObject::connect(&reader, &BarcodeReader::foundBarcodes, &printBarcodes);
	QObject::connect(&reader, &BarcodeReader::foundNoBarcodes, []() {
		qDebug() << "No barcodes found";
	});

	reader.read(image);

	return 0;
#endif
}

#include "moc_ZXingQt.cpp"
