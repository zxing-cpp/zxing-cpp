/*
 * Copyright 2023 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ZXingQt.h"

#include <QDebug>
#include <QImage>

using namespace ZXingQt;

int main(int argc, char* argv[])
{
	if (argc != 4) {
		qDebug() << "usage: ZXingQtWriter <format> <text> <filename>";
		return 1;
	}

	auto format = BarcodeFormatFromString(QString::fromUtf8(argv[1]));
	auto text = QString::fromUtf8(argv[2]);
	auto filename = QString::fromUtf8(argv[3]);

	auto barcode = Barcode::fromText(text, format, QStringLiteral("ecLevel=50%"));
	auto image = barcode.toImage(WriterOptions().scale(4));
	image.save(filename);

	return 0;
}
