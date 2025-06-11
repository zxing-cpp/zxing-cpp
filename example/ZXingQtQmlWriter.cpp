// SPDX-License-Identifier: Apache-2.0
#include "ZXingQtQmlWriter.h"
#include "BarcodeFormat.h"
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#include <QPainter>
#include <algorithm>

ZXingQtQmlWriter::ZXingQtQmlWriter(QQuickPaintedItem* parent) : QQuickPaintedItem{parent}
{}


void ZXingQtQmlWriter::generateBarcode(const QString &text)
{
	auto writer = ZXing::MultiFormatWriter(ZXing::BarcodeFormat::QRCode);
	auto matrix = writer.encode(text.toStdString(), 0, 0);
	auto bitmap = ZXing::ToMatrix<uint8_t>(matrix);

	m_qrCode = QImage(bitmap.data(), bitmap.width(), bitmap.height(), bitmap.width(), QImage::Format::Format_Grayscale8).copy();
	this->update();
}

void ZXingQtQmlWriter::paint(QPainter *painter)
{
	//Ensuring that QR code is drawn as a rectangle
	qreal minSize = std::min(boundingRect().width(), boundingRect().height());
	QRectF scaledRect{0,0,minSize,minSize};
	// Center the scaled image within the container
	scaledRect.moveCenter(boundingRect().center());

	painter->drawImage(scaledRect, m_qrCode);
}
