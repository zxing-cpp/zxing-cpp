/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingQtReader.h"

#include <QDebug>

using namespace ZXingQt;

int main(int argc, char* argv[])
{
	if (argc != 2) {
		qDebug() << "Please supply exactly one image filename";
		return 1;
	}

	QString filePath = argv[1];

	QImage image = QImage(filePath);

	if (image.isNull()) {
		qDebug() << "Could not load the filename as an image:" << filePath;
		return 1;
	}

	auto options = ReaderOptions()
					 .setFormats(BarcodeFormat::MatrixCodes)
					 .setTryInvert(false)
					 .setTextMode(TextMode::HRI)
					 .setMaxNumberOfSymbols(10);

	auto barcodes = ReadBarcodes(image, options);

	for (auto& barcode : barcodes) {
		qDebug() << "Text:   " << barcode.text();
		qDebug() << "Format: " << barcode.format();
		qDebug() << "Content:" << barcode.contentType();
		qDebug() << "";
	}

	return barcodes.isEmpty() ? 1 : 0;
}
