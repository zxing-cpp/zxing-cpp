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
					 .setTryRotate(false)
					 .setTextMode(TextMode::HRI)
					 .setMaxNumberOfSymbols(10);

	auto results = ReadBarcodes(image, options);

	for (auto& result : results) {
		qDebug() << "Text:   " << result.text();
		qDebug() << "Format: " << result.format();
		qDebug() << "Content:" << result.contentType();
		qDebug() << "";
	}

	return results.isEmpty() ? 1 : 0;
}
