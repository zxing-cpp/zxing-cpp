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

#include "ReadBarcode.h"

#include <QDebug>
#include <QImage>

// This is some sample code to start a discussion about how a minimal and header-only Qt wrapper/helper could look like.

namespace ZXing {
namespace Qt {

using ZXing::DecodeHints;
using ZXing::BarcodeFormat;
using ZXing::BarcodeFormats;
using ZXing::Binarizer;

template <typename T, typename _ = decltype(ToString(T()))>
QDebug operator<<(QDebug dbg, const T& v)
{
	return dbg.noquote() << QString::fromStdString(ToString(v));
}

class Result : private ZXing::Result
{
public:
	explicit Result(ZXing::Result&& r) : ZXing::Result(std::move(r)) {}

	using ZXing::Result::format;
	using ZXing::Result::isValid;
	using ZXing::Result::status;

	inline QString text() const { return QString::fromWCharArray(ZXing::Result::text().c_str()); }
};

Result ReadBarcode(const QImage& img, const DecodeHints& hints = {})
{
	auto ImgFmtFromQImg = [](const QImage& img) {
		switch (img.format()) {
		case QImage::Format_ARGB32:
		case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
			return ImageFormat::BGRX;
#else
			return ImageFormat::XRGB;
#endif
		case QImage::Format_RGB888: return ImageFormat::RGB;
		case QImage::Format_RGBX8888:
		case QImage::Format_RGBA8888: return ImageFormat::RGBX;
		case QImage::Format_Grayscale8: return ImageFormat::Lum;
		default: return ImageFormat::None;
		}
	};

	auto exec = [&](const QImage& img) {
		return Result(ZXing::ReadBarcode({img.bits(), img.width(), img.height(), ImgFmtFromQImg(img)}, hints));
	};

	return ImgFmtFromQImg(img) == ImageFormat::None ? exec(img.convertToFormat(QImage::Format_RGBX8888)) : exec(img);
}

} // namespace Qt
} // namespace ZXing

using namespace ZXing::Qt;

int main(int argc, char* argv[])
{
	QString filePath = argv[1];

	auto hints = DecodeHints()
					 .setFormats(BarcodeFormat::QR_CODE | BarcodeFormat::DATA_MATRIX)
					 .setTryRotate(false)
					 .setBinarizer(Binarizer::FixedThreshold);

	auto result = ReadBarcode(QImage(filePath), hints);

	qDebug() << "Text:   " << result.text();
	qDebug() << "Format: " << result.format();
	qDebug() << "Error:  " << result.status();

	return result.isValid() ? 0 : 1;
}
