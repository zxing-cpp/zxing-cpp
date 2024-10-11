/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ReadBarcode.h"

#include <QImage>
#include <QDebug>
#include <QMetaType>
#include <QScopeGuard>

#ifdef QT_MULTIMEDIA_LIB
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAbstractVideoFilter>
#else
#include <QVideoFrame>
#include <QVideoSink>
#endif
#include <QElapsedTimer>
#endif

// This is some sample code to start a discussion about how a minimal and header-only Qt wrapper/helper could look like.

namespace ZXingQt {

Q_NAMESPACE

//TODO: find a better way to export these enums to QML than to duplicate their definition
// #ifdef Q_MOC_RUN produces meta information in the moc output but it does end up working in qml
#ifdef QT_QML_LIB
enum class BarcodeFormat
{
	None            = 0,         ///< Used as a return value if no valid barcode has been detected
	Aztec           = (1 << 0),  ///< Aztec
	Codabar         = (1 << 1),  ///< Codabar
	Code39          = (1 << 2),  ///< Code39
	Code93          = (1 << 3),  ///< Code93
	Code128         = (1 << 4),  ///< Code128
	DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
	DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
	DataMatrix      = (1 << 7),  ///< DataMatrix
	EAN8            = (1 << 8),  ///< EAN-8
	EAN13           = (1 << 9),  ///< EAN-13
	ITF             = (1 << 10), ///< ITF (Interleaved Two of Five)
	MaxiCode        = (1 << 11), ///< MaxiCode
	PDF417          = (1 << 12), ///< PDF417 or
	QRCode          = (1 << 13), ///< QR Code
	UPCA            = (1 << 14), ///< UPC-A
	UPCE            = (1 << 15), ///< UPC-E
	MicroQRCode     = (1 << 16), ///< Micro QR Code
	RMQRCode        = (1 << 17), ///< Rectangular Micro QR Code
	DXFilmEdge      = (1 << 18), ///< DX Film Edge Barcode
	DataBarLimited  = (1 << 19), ///< GS1 DataBar Limited

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | DataBarLimited | DXFilmEdge | UPCA | UPCE,
	MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode | RMQRCode,
};

enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };

enum class TextMode { Plain, ECI, HRI, Hex, Escaped };

#else
using ZXing::BarcodeFormat;
using ZXing::ContentType;
using ZXing::TextMode;
#endif

using ZXing::ReaderOptions;
using ZXing::Binarizer;
using ZXing::BarcodeFormats;

Q_ENUM_NS(BarcodeFormat)
Q_ENUM_NS(ContentType)
Q_ENUM_NS(TextMode)

template<typename T, typename = decltype(ZXing::ToString(T()))>
QDebug operator<<(QDebug dbg, const T& v)
{
	return dbg.noquote() << QString::fromStdString(ToString(v));
}

class Position : public ZXing::Quadrilateral<QPoint>
{
	Q_GADGET

	Q_PROPERTY(QPoint topLeft READ topLeft)
	Q_PROPERTY(QPoint topRight READ topRight)
	Q_PROPERTY(QPoint bottomRight READ bottomRight)
	Q_PROPERTY(QPoint bottomLeft READ bottomLeft)

	using Base = ZXing::Quadrilateral<QPoint>;

public:
	using Base::Base;
};

class Barcode : private ZXing::Barcode
{
	Q_GADGET

	Q_PROPERTY(BarcodeFormat format READ format)
	Q_PROPERTY(QString formatName READ formatName)
	Q_PROPERTY(QString text READ text)
	Q_PROPERTY(QByteArray bytes READ bytes)
	Q_PROPERTY(bool isValid READ isValid)
	Q_PROPERTY(ContentType contentType READ contentType)
	Q_PROPERTY(QString contentTypeName READ contentTypeName)
	Q_PROPERTY(Position position READ position)

	QString _text;
	QByteArray _bytes;
	Position _position;

public:
	Barcode() = default; // required for qmetatype machinery

	explicit Barcode(ZXing::Barcode&& r) : ZXing::Barcode(std::move(r)) {
		_text = QString::fromStdString(ZXing::Barcode::text());
		_bytes = QByteArray(reinterpret_cast<const char*>(ZXing::Barcode::bytes().data()), Size(ZXing::Barcode::bytes()));
		auto& pos = ZXing::Barcode::position();
		auto qp = [&pos](int i) { return QPoint(pos[i].x, pos[i].y); };
		_position = {qp(0), qp(1), qp(2), qp(3)};
	}

	using ZXing::Barcode::isValid;

	BarcodeFormat format() const { return static_cast<BarcodeFormat>(ZXing::Barcode::format()); }
	ContentType contentType() const { return static_cast<ContentType>(ZXing::Barcode::contentType()); }
	QString formatName() const { return QString::fromStdString(ZXing::ToString(ZXing::Barcode::format())); }
	QString contentTypeName() const { return QString::fromStdString(ZXing::ToString(ZXing::Barcode::contentType())); }
	const QString& text() const { return _text; }
	const QByteArray& bytes() const { return _bytes; }
	const Position& position() const { return _position; }
};

inline QList<Barcode> ZXBarcodesToQBarcodes(ZXing::Barcodes&& zxres)
{
	QList<Barcode> res;
	for (auto&& r : zxres)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		res.push_back(Barcode(std::move(r)));
#else
		res.emplace_back(std::move(r));
#endif
	return res;
}

inline QList<Barcode> ReadBarcodes(const QImage& img, const ReaderOptions& opts = {})
{
	using namespace ZXing;

	auto ImgFmtFromQImg = [](const QImage& img) {
		switch (img.format()) {
		case QImage::Format_ARGB32:
		case QImage::Format_RGB32:
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
			return ImageFormat::BGRA;
#else
			return ImageFormat::ARGB;
#endif
		case QImage::Format_RGB888: return ImageFormat::RGB;
		case QImage::Format_RGBX8888:
		case QImage::Format_RGBA8888: return ImageFormat::RGBA;
		case QImage::Format_Grayscale8: return ImageFormat::Lum;
		default: return ImageFormat::None;
		}
	};

	auto exec = [&](const QImage& img) {
		return ZXBarcodesToQBarcodes(ZXing::ReadBarcodes(
			{img.bits(), img.width(), img.height(), ImgFmtFromQImg(img), static_cast<int>(img.bytesPerLine())}, opts));
	};

	return ImgFmtFromQImg(img) == ImageFormat::None ? exec(img.convertToFormat(QImage::Format_Grayscale8)) : exec(img);
}

inline Barcode ReadBarcode(const QImage& img, const ReaderOptions& opts = {})
{
	auto res = ReadBarcodes(img, ReaderOptions(opts).setMaxNumberOfSymbols(1));
	return !res.isEmpty() ? res.takeFirst() : Barcode();
}

#ifdef QT_MULTIMEDIA_LIB
inline QList<Barcode> ReadBarcodes(const QVideoFrame& frame, const ReaderOptions& opts = {})
{
	using namespace ZXing;

	ImageFormat fmt = ImageFormat::None;
	int pixStride = 0;
	int pixOffset = 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define FORMAT(F5, F6) QVideoFrame::Format_##F5
#define FIRST_PLANE
#else
#define FORMAT(F5, F6) QVideoFrameFormat::Format_##F6
#define FIRST_PLANE 0
#endif

	switch (frame.pixelFormat()) {
	case FORMAT(ARGB32, ARGB8888):
	case FORMAT(ARGB32_Premultiplied, ARGB8888_Premultiplied):
	case FORMAT(RGB32, RGBX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::BGRA;
#else
		fmt = ImageFormat::ARGB;
#endif
		break;

	case FORMAT(BGRA32, BGRA8888):
	case FORMAT(BGRA32_Premultiplied, BGRA8888_Premultiplied):
	case FORMAT(BGR32, BGRX8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::RGBA;
#else
		fmt = ImageFormat::ABGR;
#endif
		break;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	case QVideoFrame::Format_RGB24: fmt = ImageFormat::RGB; break;
	case QVideoFrame::Format_BGR24: fmt = ImageFormat::BGR; break;
	case QVideoFrame::Format_YUV444: fmt = ImageFormat::Lum, pixStride = 3; break;
#else
	case QVideoFrameFormat::Format_P010:
	case QVideoFrameFormat::Format_P016: fmt = ImageFormat::Lum, pixStride = 1; break;
#endif

	case FORMAT(AYUV444, AYUV):
	case FORMAT(AYUV444_Premultiplied, AYUV_Premultiplied):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 3;
#else
		fmt = ImageFormat::Lum, pixStride = 4, pixOffset = 2;
#endif
		break;

	case FORMAT(YUV420P, YUV420P):
	case FORMAT(NV12, NV12):
	case FORMAT(NV21, NV21):
	case FORMAT(IMC1, IMC1):
	case FORMAT(IMC2, IMC2):
	case FORMAT(IMC3, IMC3):
	case FORMAT(IMC4, IMC4):
	case FORMAT(YV12, YV12): fmt = ImageFormat::Lum; break;
	case FORMAT(UYVY, UYVY): fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;
	case FORMAT(YUYV, YUYV): fmt = ImageFormat::Lum, pixStride = 2; break;

	case FORMAT(Y8, Y8): fmt = ImageFormat::Lum; break;
	case FORMAT(Y16, Y16): fmt = ImageFormat::Lum, pixStride = 2, pixOffset = 1; break;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
	case FORMAT(ABGR32, ABGR8888):
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
		fmt = ImageFormat::RGBA;
#else
		fmt = ImageFormat::ABGR;
#endif
		break;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	case FORMAT(YUV422P, YUV422P): fmt = ImageFormat::Lum; break;
#endif
	default: break;
	}

	if (fmt != ImageFormat::None) {
		auto img = frame; // shallow copy just get access to non-const map() function
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		if (!img.isValid() || !img.map(QAbstractVideoBuffer::ReadOnly)){
#else
		if (!img.isValid() || !img.map(QVideoFrame::ReadOnly)){
#endif
			qWarning() << "invalid QVideoFrame: could not map memory";
			return {};
		}
		QScopeGuard unmap([&] { img.unmap(); });

		return ZXBarcodesToQBarcodes(ZXing::ReadBarcodes(
			{img.bits(FIRST_PLANE) + pixOffset, img.width(), img.height(), fmt, img.bytesPerLine(FIRST_PLANE), pixStride}, opts));
	}
	else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		if (QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat()) != QImage::Format_Invalid) {
			qWarning() << "unsupported QVideoFrame::pixelFormat";
			return {};
		}
		auto qimg = frame.image();
#else
		auto qimg = frame.toImage();
#endif
		if (qimg.format() != QImage::Format_Invalid)
			return ReadBarcodes(qimg, opts);
		qWarning() << "failed to convert QVideoFrame to QImage";
		return {};
	}
}

inline Barcode ReadBarcode(const QVideoFrame& frame, const ReaderOptions& opts = {})
{
	auto res = ReadBarcodes(frame, ReaderOptions(opts).setMaxNumberOfSymbols(1));
	return !res.isEmpty() ? res.takeFirst() : Barcode();
}

#define ZQ_PROPERTY(Type, name, setter) \
public: \
	Q_PROPERTY(Type name READ name WRITE setter NOTIFY name##Changed) \
	Type name() const noexcept { return ReaderOptions::name(); } \
	Q_SLOT void setter(const Type& newVal) \
	{ \
		if (name() != newVal) { \
			ReaderOptions::setter(newVal); \
			emit name##Changed(); \
		} \
	} \
	Q_SIGNAL void name##Changed();


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class BarcodeReader : public QAbstractVideoFilter, private ReaderOptions
#else
class BarcodeReader : public QObject, private ReaderOptions
#endif
{
	Q_OBJECT

public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	BarcodeReader(QObject* parent = nullptr) : QAbstractVideoFilter(parent) {}
#else
	BarcodeReader(QObject* parent = nullptr) : QObject(parent) {}
#endif

	// TODO: find out how to properly expose QFlags to QML
	// simply using ZQ_PROPERTY(BarcodeFormats, formats, setFormats)
	// results in the runtime error "can't assign int to formats"
	Q_PROPERTY(int formats READ formats WRITE setFormats NOTIFY formatsChanged)
	int formats() const noexcept
	{
		auto fmts = ReaderOptions::formats();
		return *reinterpret_cast<int*>(&fmts);
	}
	Q_SLOT void setFormats(int newVal)
	{
		if (formats() != newVal) {
			ReaderOptions::setFormats(static_cast<ZXing::BarcodeFormat>(newVal));
			emit formatsChanged();
			qDebug() << ReaderOptions::formats();
		}
	}
	Q_SIGNAL void formatsChanged();

	Q_PROPERTY(TextMode textMode READ textMode WRITE setTextMode NOTIFY textModeChanged)
	TextMode textMode() const noexcept { return static_cast<TextMode>(ReaderOptions::textMode()); }
	Q_SLOT void setTextMode(TextMode newVal)
	{
		if (textMode() != newVal) {
			ReaderOptions::setTextMode(static_cast<ZXing::TextMode>(newVal));
			emit textModeChanged();
		}
	}
	Q_SIGNAL void textModeChanged();

	ZQ_PROPERTY(bool, tryRotate, setTryRotate)
	ZQ_PROPERTY(bool, tryHarder, setTryHarder)
	ZQ_PROPERTY(bool, tryInvert, setTryInvert)
	ZQ_PROPERTY(bool, tryDownscale, setTryDownscale)
	ZQ_PROPERTY(bool, isPure, setIsPure)

	// For debugging/development
	int runTime = 0;
	Q_PROPERTY(int runTime MEMBER runTime)

public slots:
	ZXingQt::Barcode process(const QVideoFrame& image)
	{
		QElapsedTimer t;
		t.start();

		auto res = ReadBarcode(image, *this);

		runTime = t.elapsed();

		if (res.isValid())
			emit foundBarcode(res);
		else
			emit failedRead();
		return res;
	}

signals:
	void failedRead();
	void foundBarcode(ZXingQt::Barcode barcode);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
public:
	QVideoFilterRunnable *createFilterRunnable() override;
#else
private:
	QVideoSink *_sink = nullptr;

public:
	void setVideoSink(QVideoSink* sink) {
		if (_sink == sink)
			return;

		if (_sink)
			disconnect(_sink, nullptr, this, nullptr);

		_sink = sink;
		connect(_sink, &QVideoSink::videoFrameChanged, this, &BarcodeReader::process);
	}
	Q_PROPERTY(QVideoSink* videoSink MEMBER _sink WRITE setVideoSink)
#endif

};

#undef ZX_PROPERTY

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class VideoFilterRunnable : public QVideoFilterRunnable
{
	BarcodeReader* _filter = nullptr;

public:
	explicit VideoFilterRunnable(BarcodeReader* filter) : _filter(filter) {}

	QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat& /*surfaceFormat*/, RunFlags /*flags*/) override
	{
		_filter->process(*input);
		return *input;
	}
};

inline QVideoFilterRunnable* BarcodeReader::createFilterRunnable()
{
	return new VideoFilterRunnable(this);
}
#endif

#endif // QT_MULTIMEDIA_LIB

} // namespace ZXingQt


Q_DECLARE_METATYPE(ZXingQt::Position)
Q_DECLARE_METATYPE(ZXingQt::Barcode)

#ifdef QT_QML_LIB

#include <QQmlEngine>

namespace ZXingQt {

inline void registerQmlAndMetaTypes()
{
	qRegisterMetaType<ZXingQt::BarcodeFormat>("BarcodeFormat");
	qRegisterMetaType<ZXingQt::ContentType>("ContentType");
	qRegisterMetaType<ZXingQt::TextMode>("TextMode");

	// supposedly the Q_DECLARE_METATYPE should be used with the overload without a custom name
	// but then the qml side complains about "unregistered type"
	qRegisterMetaType<ZXingQt::Position>("Position");
	qRegisterMetaType<ZXingQt::Barcode>("Barcode");

	qmlRegisterUncreatableMetaObject(
		ZXingQt::staticMetaObject, "ZXing", 1, 0, "ZXing", "Access to enums & flags only");
	qmlRegisterType<ZXingQt::BarcodeReader>("ZXing", 1, 0, "BarcodeReader");
}

} // namespace ZXingQt

#endif // QT_QML_LIB
