/*
 * Copyright 2020 Axel Waggershauser
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
