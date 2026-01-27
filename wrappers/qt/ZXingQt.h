/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ZXingCpp.h"

#include <QImage>
#include <QDebug>
#include <QMetaType>
#include <QPoint>
#include <QVector>
#include <QObject>
#include <QScopeGuard>
#include <QThreadPool>
#include <QtQmlIntegration/qqmlintegration.h>

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

namespace Detail {

template<typename Cout, typename Cin>
inline Cout transcode(Cin&& in)
{
	Cout out;
	out.reserve(in.size());
	for (auto&& v : in)
		out.push_back(static_cast<typename Cout::value_type>(std::move(v)));
	return out;
}

inline std::string_view qba2sv(const QByteArray& ba) noexcept
{
	return std::string_view(ba.constData(), static_cast<size_t>(ba.size()));
}

} // namespace Detail


enum class BarcodeFormat : unsigned int
{
#define X(NAME, SYM, VAR, FLAGS, ZINT, ENABLED, HRI) NAME = ZX_BCF_ID(SYM, VAR),
	ZX_BCF_LIST(X)
#undef X
};

enum class ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };
enum class TextMode { Plain, ECI, HRI, Escaped, Hex, HexECI };
enum class Binarizer { LocalAverage, GlobalHistogram, FixedThreshold, BoolCast };

Q_ENUM_NS(BarcodeFormat)
Q_ENUM_NS(ContentType)
Q_ENUM_NS(TextMode)
Q_ENUM_NS(Binarizer)

using BarcodeFormats = QVector<BarcodeFormat>;

using ZXing::ReaderOptions;
using ZXing::WriterOptions;

inline BarcodeFormat BarcodeFormatFromString(QStringView str)
{
	return static_cast<BarcodeFormat>(ZXing::BarcodeFormatFromString(Detail::qba2sv(str.toUtf8())));
}

inline BarcodeFormats BarcodeFormatsFromString(QStringView str)
{
	return Detail::transcode<BarcodeFormats>(ZXing::BarcodeFormatsFromString(Detail::qba2sv(str.toUtf8())));
}

inline BarcodeFormats ListBarcodeFormats(BarcodeFormat filter = BarcodeFormat::None)
{
	return Detail::transcode<BarcodeFormats>(ZXing::BarcodeFormats::list(
		filter == BarcodeFormat::None ? ZXing::BarcodeFormats() : static_cast<ZXing::BarcodeFormat>(filter)));
}

#define ZQ_DECL_TO_STRING(TYPE) \
	inline QString ToString(::ZXingQt::TYPE v) \
	{ \
		return QString::fromStdString(::ZXing::ToString(static_cast<::ZXing::TYPE>(v))); \
	}

ZQ_DECL_TO_STRING(BarcodeFormat)
ZQ_DECL_TO_STRING(ContentType)
#undef ZQ_DECL_TO_STRING

// template <typename T, typename = decltype(ToString(T()))>
// QDebug operator<<(QDebug dbg, const T& v)
// {
// 	return dbg.noquote() << ToString(v);
// }

class Position : public ZXing::Quadrilateral<QPoint>
{
	Q_GADGET

	Q_PROPERTY(QPoint topLeft READ topLeft)
	Q_PROPERTY(QPoint topRight READ topRight)
	Q_PROPERTY(QPoint bottomRight READ bottomRight)
	Q_PROPERTY(QPoint bottomLeft READ bottomLeft)
	Q_PROPERTY(QPoint center READ center)

	using Base = ZXing::Quadrilateral<QPoint>;

public:
	using Base::Base;

	QPoint center() const { return std::accumulate(this->begin(), this->end(), QPoint(0, 0)) / 4; }
};

class Barcode : private ZXing::Barcode
{
	Q_GADGET

	Q_PROPERTY(BarcodeFormat format READ format)
	Q_PROPERTY(QString text READ text)
	Q_PROPERTY(QByteArray bytes READ bytes)
	Q_PROPERTY(bool isValid READ isValid)
	Q_PROPERTY(ContentType contentType READ contentType)
	Q_PROPERTY(Position position READ position)

public:
	Barcode() = default; // required for qmetatype machinery

	explicit Barcode(ZXing::Barcode&& r) : ZXing::Barcode(std::move(r)) {}

	using ZXing::Barcode::isValid;

	BarcodeFormat format() const { return static_cast<BarcodeFormat>(ZXing::Barcode::format()); }
	ContentType contentType() const { return static_cast<ContentType>(ZXing::Barcode::contentType()); }
	QString text() const { return QString::fromStdString(ZXing::Barcode::text()); }

	QByteArray bytes() const
	{
		return QByteArray(reinterpret_cast<const char*>(ZXing::Barcode::bytes().data()), std::size(ZXing::Barcode::bytes()));
	}

	Position position() const
	{
		auto& pos = ZXing::Barcode::position();
		auto qp = [&pos](int i) { return QPoint(pos[i].x, pos[i].y); };
		return {qp(0), qp(1), qp(2), qp(3)};
	}

	QString toSVG(const WriterOptions& options = {}) const
	{
		return QString::fromStdString(ZXing::WriteBarcodeToSVG(*this, options));
	}

	QImage toImage(const WriterOptions& options = {}) const
	{
		auto img = ZXing::WriteBarcodeToImage(*this, options);
		return QImage(img.data(), img.width(), img.height(), img.width(), QImage::Format::Format_Grayscale8).copy();
	}

	static Barcode fromText(QStringView text, BarcodeFormat format, QStringView options = {})
	{
		auto opts = ZXing::CreatorOptions(static_cast<ZXing::BarcodeFormat>(format), options.toUtf8().toStdString());
		return Barcode(ZXing::CreateBarcodeFromText(Detail::qba2sv(text.toUtf8()), opts));
	}

	static Barcode fromBytes(const QByteArray& bytes, BarcodeFormat format, QStringView options = {})
	{
		auto opts = ZXing::CreatorOptions(static_cast<ZXing::BarcodeFormat>(format), options.toUtf8().toStdString());
		return Barcode(ZXing::CreateBarcodeFromBytes(bytes, opts));
	}

};

inline QVector<Barcode> ReadBarcodes(const QImage& img, const ReaderOptions& opts = {})
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
		return Detail::transcode<QVector<Barcode>>(ZXing::ReadBarcodes(
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
inline QVector<Barcode> ReadBarcodes(const QVideoFrame& frame, const ReaderOptions& opts = {})
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

		return Detail::transcode<QVector<Barcode>>(ZXing::ReadBarcodes(
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
#endif // QT_MULTIMEDIA_LIB

#define ZQ_PROPERTY(Type, name, setter) \
public: \
	Q_PROPERTY(Type name READ name WRITE setter NOTIFY name##Changed) \
	Type name() const noexcept { return ReaderOptions::name(); } \
	Q_SLOT void setter(const Type& newVal) \
	{ \
		if (name() != newVal) { \
			ReaderOptions::setter(newVal); \
			Q_EMIT name##Changed(); \
		} \
	} \
	Q_SIGNAL void name##Changed();


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(QT_MULTIMEDIA_LIB)
class BarcodeReader : public QAbstractVideoFilter, private ReaderOptions
#else
class BarcodeReader : public QObject, private ReaderOptions
#endif
{
	Q_OBJECT

	QThreadPool _pool;

	Q_PROPERTY(BarcodeFormats formats READ formats WRITE setFormats NOTIFY formatsChanged)
	Q_PROPERTY(TextMode textMode READ textMode WRITE setTextMode NOTIFY textModeChanged)

	void emitFoundBarcodes(const QVector<Barcode>& barcodes)
	{
		if (!barcodes.isEmpty())
			Q_EMIT foundBarcodes(barcodes);
		else
			Q_EMIT foundNoBarcodes();
	}

public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(QT_MULTIMEDIA_LIB)
	BarcodeReader(QObject* parent = nullptr) : QAbstractVideoFilter(parent) {}
#else
	BarcodeReader(QObject* parent = nullptr) : QObject(parent)
#endif
	{
		_pool.setMaxThreadCount(1);
	}

	~BarcodeReader()
	{
		_pool.setMaxThreadCount(0);
		_pool.waitForDone(-1);
	}

	Q_PROPERTY(int maxThreadCount READ maxThreadCount WRITE setMaxThreadCount)
	int maxThreadCount() const { return _pool.maxThreadCount(); }
	Q_SLOT void setMaxThreadCount(int maxThreadCount)
	{
		if (_pool.maxThreadCount() != maxThreadCount) {
			_pool.setMaxThreadCount(maxThreadCount);
			Q_EMIT maxThreadCountChanged();
		}
	}
	Q_SIGNAL void maxThreadCountChanged();

	BarcodeFormats formats() const noexcept { return Detail::transcode<BarcodeFormats>(ReaderOptions::formats()); }
	Q_SLOT void setFormats(const BarcodeFormats& newVal)
	{
		if (formats() != newVal) {
			ReaderOptions::setFormats(Detail::transcode<std::vector<ZXing::BarcodeFormat>>(newVal));
			Q_EMIT formatsChanged();
		}
	}
	Q_SIGNAL void formatsChanged();

	TextMode textMode() const noexcept { return static_cast<TextMode>(ReaderOptions::textMode()); }
	Q_SLOT void setTextMode(TextMode newVal)
	{
		if (textMode() != newVal) {
			ReaderOptions::setTextMode(static_cast<ZXing::TextMode>(newVal));
			Q_EMIT textModeChanged();
		}
	}
	Q_SIGNAL void textModeChanged();

	// ZQ_PROPERTY(BarcodeFormats, formats, setFormats)
	// ZQ_PROPERTY(TextMode, textMode, setTextMode)
	ZQ_PROPERTY(bool, tryRotate, setTryRotate)
	ZQ_PROPERTY(bool, tryHarder, setTryHarder)
	ZQ_PROPERTY(bool, tryInvert, setTryInvert)
	ZQ_PROPERTY(bool, tryDownscale, setTryDownscale)
	ZQ_PROPERTY(bool, isPure, setIsPure)

	// For debugging/development
	QAtomicInt runTime = 0;
	Q_PROPERTY(int runTime MEMBER runTime)

	Q_SLOT QVector<Barcode> read(const QImage& image)
	{
		auto barcodes = ReadBarcodes(image, *this);
		emitFoundBarcodes(barcodes);
		return barcodes;
	}

Q_SIGNALS:
	void foundNoBarcodes();
	void foundBarcodes(const QVector<Barcode>& barcodes);

public:
#ifdef QT_MULTIMEDIA_LIB
	// Function should be thread safe, as it may be called from a separate thread.
	Q_SLOT QVector<Barcode> read(const QVideoFrame& image)
	{
		QElapsedTimer t;
		t.start();
		auto barcodes = ReadBarcodes(image, *this);
		runTime = t.elapsed();
		emitFoundBarcodes(barcodes);
		return barcodes;
	}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
public:
	QVideoFilterRunnable *createFilterRunnable() override;
#else
private:
	QVideoSink *_sink = nullptr;

public:
	void setVideoSink(QVideoSink* sink)
	{
		if (_sink == sink)
			return;

		if (_sink)
			disconnect(_sink, nullptr, this, nullptr);

		_sink = sink;
		connect(_sink, &QVideoSink::videoFrameChanged, this, &BarcodeReader::onVideoFrameChanged, Qt::DirectConnection);
	}
	void onVideoFrameChanged(const QVideoFrame& frame)
	{
		if (_pool.activeThreadCount() >= _pool.maxThreadCount())
			return; // we are busy => skip the frame

		_pool.start([this, frame]() { read(frame); });
	}
	Q_PROPERTY(QVideoSink* videoSink MEMBER _sink WRITE setVideoSink)

#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#endif // QT_MULTIMEDIA_LIB

};


#undef ZQ_PROPERTY

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(QT_MULTIMEDIA_LIB)
class VideoFilterRunnable : public QVideoFilterRunnable
{
	BarcodeReader* _filter = nullptr;

public:
	explicit VideoFilterRunnable(BarcodeReader* filter) : _filter(filter) {}

	QVideoFrame run(QVideoFrame* input, const QVideoSurfaceFormat& /*surfaceFormat*/, RunFlags /*flags*/) override
	{
		_filter->read(*input);
		return *input;
	}
};

inline QVideoFilterRunnable* BarcodeReader::createFilterRunnable()
{
	return new VideoFilterRunnable(this);
}
#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0) && defined(QT_MULTIMEDIA_LIB)

} // namespace ZXingQt


// Q_DECLARE_METATYPE: compile-time declaration required for QVariant storage and template instantiation
Q_DECLARE_METATYPE(ZXingQt::Position)
Q_DECLARE_METATYPE(ZXingQt::Barcode)

#ifdef QT_QML_LIB

#include <QQmlEngine>

namespace ZXingQt {

class ZXingQml : public QObject
{
	Q_OBJECT
public:
	Q_INVOKABLE static QString FormatToString(BarcodeFormat format) { return ZXingQt::ToString(format); }
	Q_INVOKABLE static QString ContentTypeToString(ContentType type) { return ZXingQt::ToString(type); }
	Q_INVOKABLE static QVector<BarcodeFormat> ListBarcodeFormats(BarcodeFormat filter = BarcodeFormat::None)
	{
		return ZXingQt::ListBarcodeFormats(filter);
	}
};

namespace Detail {

inline void registerQmlAndMetaTypes()
{
	// qRegisterMetaType: runtime registration required for queued signal/slot connections across threads
	qRegisterMetaType<ZXingQt::BarcodeFormat>("BarcodeFormat");
	qRegisterMetaType<ZXingQt::ContentType>("ContentType");
	qRegisterMetaType<ZXingQt::TextMode>("TextMode");
	qRegisterMetaType<ZXingQt::Position>("Position");
	qRegisterMetaType<ZXingQt::Barcode>("Barcode");
	qRegisterMetaType<QVector<ZXingQt::BarcodeFormat>>("QVector<BarcodeFormat>");
	qRegisterMetaType<QVector<ZXingQt::Barcode>>("QVector<Barcode>");

	// Custom enum to string converters
	QMetaType::registerConverter<BarcodeFormat, QString>(
		[](BarcodeFormat format) { return QString::fromStdString(ZXing::ToString(static_cast<ZXing::BarcodeFormat>(format))); });

	QMetaType::registerConverter<ContentType, QString>(
		[](ContentType type) { return QString::fromStdString(ZXing::ToString(static_cast<ZXing::ContentType>(type))); });

	// qmlRegisterType allows us to store the position / barcode in a QML property, i.e. property barcode myBarcode: ...
	qmlRegisterType<ZXingQt::Position>("ZXing", 1, 0, "position");
	qmlRegisterType<ZXingQt::Barcode>("ZXing", 1, 0, "barcode");

	qmlRegisterUncreatableMetaObject(ZXingQt::staticMetaObject, "ZXing", 1, 0, "ZXing", QStringLiteral("Access to enums only"));
	qmlRegisterType<ZXingQt::BarcodeReader>("ZXing", 1, 0, "BarcodeReader");

	qmlRegisterSingletonType<ZXingQml>("ZXing", 1, 0, "ZXingQml", [](QQmlEngine*, QJSEngine*) -> QObject* { return new ZXingQml(); });
}

struct ZXingQtInitializer
{
	ZXingQtInitializer() { registerQmlAndMetaTypes(); }
} inline zxingQtInitializer;

} // namespace Detail

} // namespace ZXingQt

#endif // QT_QML_LIB
