/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#pragma warning(disable : 4996)
#endif

#include "BarcodeReader.h"

#include "BarcodeFormat.h"
#include "ReaderOptions.h"
#include "ReadBarcode.h"
#include "ReadResult.h"
#include "Utf.h"

#include <algorithm>
#include <MemoryBuffer.h>
#include <stdexcept>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;

namespace ZXing {

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types)
{
	init(tryHarder, tryRotate, types);
}

BarcodeReader::BarcodeReader(bool tryHarder, bool tryRotate)
{
	init(tryHarder, tryRotate, nullptr);
}

BarcodeReader::BarcodeReader(bool tryHarder)
{
	init(tryHarder, tryHarder, nullptr);
}

void
BarcodeReader::init(bool tryHarder, bool tryRotate, const Platform::Array<BarcodeType>^ types)
{
	m_opts.reset(new ReaderOptions());
	m_opts->setTryHarder(tryHarder);
	m_opts->setTryRotate(tryRotate);
	m_opts->setTryInvert(tryHarder);

	if (types != nullptr && types->Length > 0) {
		BarcodeFormats barcodeFormats;
		for (BarcodeType type : types) {
			barcodeFormats |= BarcodeReader::ConvertRuntimeToNative(type);
		}
		m_opts->setFormats(barcodeFormats);
	}
}

BarcodeReader::~BarcodeReader()
{
}

BarcodeFormat BarcodeReader::ConvertRuntimeToNative(BarcodeType type)
{
	switch (type) {
	case BarcodeType::AZTEC:
		return BarcodeFormat::Aztec;
	case BarcodeType::CODABAR:
		return BarcodeFormat::Codabar;
	case BarcodeType::CODE_128:
		return BarcodeFormat::Code128;
	case BarcodeType::CODE_39:
		return BarcodeFormat::Code39;
	case BarcodeType::CODE_93:
		return BarcodeFormat::Code93;
	case BarcodeType::DATA_MATRIX:
		return BarcodeFormat::DataMatrix;
	case BarcodeType::EAN_13:
		return BarcodeFormat::EAN13;
	case BarcodeType::EAN_8:
		return BarcodeFormat::EAN8;
	case BarcodeType::ITF:
		return BarcodeFormat::ITF;
	case BarcodeType::MAXICODE:
		return BarcodeFormat::MaxiCode;
	case BarcodeType::PDF_417:
		return BarcodeFormat::PDF417;
	case BarcodeType::QR_CODE:
		return BarcodeFormat::QRCode;
	case BarcodeType::MICRO_QR_CODE:
		return BarcodeFormat::MicroQRCode;
	case BarcodeType::RMQR_CODE:
		return BarcodeFormat::RMQRCode;
	case BarcodeType::RSS_14:
		return BarcodeFormat::DataBar;
	case BarcodeType::RSS_EXPANDED:
		return BarcodeFormat::DataBarExpanded;
	case BarcodeType::DATA_BAR_LIMITED:
		return BarcodeFormat::DataBarLimited;
	case BarcodeType::DX_FILM_EDGE:
		return BarcodeFormat::DXFilmEdge;
	case BarcodeType::UPC_A:
		return BarcodeFormat::UPCA;
	case BarcodeType::UPC_E:
		return BarcodeFormat::UPCE;
	default:
		std::wstring typeAsString = type.ToString()->Begin();
		throw std::invalid_argument("Unknown Barcode Type: " + ToUtf8(typeAsString));
	}
}

BarcodeType BarcodeReader::ConvertNativeToRuntime(BarcodeFormat format)
{
	switch (format) {
	case BarcodeFormat::Aztec:
		return BarcodeType::AZTEC;
	case BarcodeFormat::Codabar:
		return BarcodeType::CODABAR;
	case BarcodeFormat::Code128:
		return BarcodeType::CODE_128;
	case BarcodeFormat::Code39:
		return BarcodeType::CODE_39;
	case BarcodeFormat::Code93:
		return BarcodeType::CODE_93;
	case BarcodeFormat::DataMatrix:
		return BarcodeType::DATA_MATRIX;
	case BarcodeFormat::EAN13:
		return BarcodeType::EAN_13;
	case BarcodeFormat::EAN8:
		return BarcodeType::EAN_8;
	case BarcodeFormat::ITF:
		return BarcodeType::ITF;
	case BarcodeFormat::MaxiCode:
		return BarcodeType::MAXICODE;
	case BarcodeFormat::PDF417:
		return BarcodeType::PDF_417;
	case BarcodeFormat::QRCode:
		return BarcodeType::QR_CODE;
	case BarcodeFormat::MicroQRCode:
		return BarcodeType::MICRO_QR_CODE;
	case BarcodeFormat::RMQRCode:
		return BarcodeType::RMQR_CODE;
	case BarcodeFormat::DataBar:
		return BarcodeType::RSS_14;
	case BarcodeFormat::DataBarExpanded:
		return BarcodeType::RSS_EXPANDED;
	case BarcodeFormat::DataBarLimited:
		return BarcodeType::DATA_BAR_LIMITED;
	case BarcodeFormat::DXFilmEdge:
		return BarcodeType::DX_FILM_EDGE;
	case BarcodeFormat::UPCA:
		return BarcodeType::UPC_A;
	case BarcodeFormat::UPCE:
		return BarcodeType::UPC_E;
	default:
		throw std::invalid_argument("Unknown Barcode Format ");
	}
}

static Platform::String^ ToPlatformString(const std::string& str)
{
	std::wstring wstr = FromUtf8(str);
	return ref new Platform::String(wstr.c_str(), (unsigned)wstr.length());
}

ReadResult^
BarcodeReader::Read(SoftwareBitmap^ bitmap, int cropWidth, int cropHeight)
{
	try {
		cropWidth = cropWidth <= 0 ? bitmap->PixelWidth : std::min(bitmap->PixelWidth, cropWidth);
		cropHeight = cropHeight <= 0 ? bitmap->PixelHeight : std::min(bitmap->PixelHeight, cropHeight);
		int cropLeft = (bitmap->PixelWidth - cropWidth) / 2;
		int cropTop = (bitmap->PixelHeight - cropHeight) / 2;

		auto inBuffer = bitmap->LockBuffer(BitmapBufferAccessMode::Read);
		auto inMemRef = inBuffer->CreateReference();
		ComPtr<IMemoryBufferByteAccess> inBufferAccess;

		if (SUCCEEDED(ComPtr<IUnknown>(reinterpret_cast<IUnknown*>(inMemRef)).As(&inBufferAccess))) {
			BYTE* inBytes = nullptr;
			UINT32 inCapacity = 0;
			inBufferAccess->GetBuffer(&inBytes, &inCapacity);

			ImageFormat fmt = ImageFormat::None;
			switch (bitmap->BitmapPixelFormat)
			{
			case BitmapPixelFormat::Gray8: fmt = ImageFormat::Lum; break;
			case BitmapPixelFormat::Bgra8: fmt = ImageFormat::BGRA; break;
			case BitmapPixelFormat::Rgba8: fmt = ImageFormat::RGBA; break;
			default:
				throw std::runtime_error("Unsupported BitmapPixelFormat");
			}

			auto img = ImageView(inBytes, bitmap->PixelWidth, bitmap->PixelHeight, fmt, inBuffer->GetPlaneDescription(0).Stride)
						   .cropped(cropLeft, cropTop, cropWidth, cropHeight);

			auto barcode = ReadBarcode(img, *m_opts);
			if (barcode.isValid()) {
				return ref new ReadResult(ToPlatformString(ZXing::ToString(barcode.format())), ToPlatformString(barcode.text()), ConvertNativeToRuntime(barcode.format()));
			}
		} else {
			throw std::runtime_error("Failed to read bitmap's data");
		}
	}
	catch (const std::exception& e) {
		OutputDebugStringA(e.what());
	}
	catch (...) {
	}
	return nullptr;
}

} // ZXing
