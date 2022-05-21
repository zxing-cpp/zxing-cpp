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

	QImage fileImage = QImage(filePath);

	if (fileImage.isNull()) {
		qDebug() << "Could not load the filename as an image:" << filePath;
		return 1;
	}

	auto hints = DecodeHints()
					 .setFormats(BarcodeFormat::QRCode)
					 .setTryRotate(false)
					 .setBinarizer(Binarizer::FixedThreshold);

	auto result = ReadBarcode(fileImage, hints);

	qDebug() << "Text:   " << result.text();
	qDebug() << "Format: " << result.format();
	qDebug() << "Error:  " << result.status();

	return result.isValid() ? 0 : 1;
}
