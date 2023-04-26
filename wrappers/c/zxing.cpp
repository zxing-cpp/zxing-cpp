#include "zxing.h"

#include <ZXing/ImageView.h>
#include <ZXing/BarcodeFormat.h>
#include <ZXing/DecodeHints.h>
#include <ZXing/Result.h>
#include <ZXing/ReadBarcode.h>

extern "C" {

/*
 * ZXing/ImageView.h
 */

zxing_ImageView * zxing_ImageView_new (const uint8_t * data, int width, int height, zxing_ImageFormat format, int rowStride, int pixStride)
{
	ZXing::ImageFormat cppformat = (ZXing::ImageFormat) format;
	return new ZXing::ImageView(data, width, height, cppformat, rowStride, pixStride);
}

void zxing_ImageView_delete (zxing_ImageView * iv)
{
	delete iv;
}

int zxing_ImageView_width (const zxing_ImageView * iv)
{
	return iv->width();
}

int zxing_ImageView_height (const zxing_ImageView * iv)
{
	return iv->height();
}

int zxing_ImageView_pixStride (const zxing_ImageView * iv)
{
	return iv->pixStride();
}

int zxing_ImageView_rowStride (const zxing_ImageView * iv)
{
	return iv->rowStride();
}

zxing_ImageFormat zxing_ImageView_format (const zxing_ImageView * iv)
{
	return (zxing_ImageFormat) iv->format();
}

/*
 * ...
 */

/*
 * ZXing/BarcodeFormat.h
 */

zxing_BarcodeFormat zxing_BarcodeFormatFromString (const char * format)
{
	return (zxing_BarcodeFormat) ZXing::BarcodeFormatFromString(std::string(format));
}

// TODO:
//zxing_BarcodeFormat zxing_BarcodeFormatsFromString (const char * formats)
//{
//    return (zxing_BarcodeFormat) ZXing::BarcodeFormatsFromString(std::string(formats));
//}

/*
 * ZXing/DecodeHints.h
 */

zxing_DecodeHints * zxing_DecodeHints_new ()
{
	return new ZXing::DecodeHints();
}

void zxing_DecodeHints_delete (zxing_DecodeHints * hints)
{
	delete hints;
}

void zxing_DecodeHints_setTryHarder (zxing_DecodeHints * hints, bool tryHarder)
{
	hints->setTryHarder(tryHarder);
}

void zxing_DecodeHints_setTryDownscale (zxing_DecodeHints * hints, bool tryDownscale)
{
	hints->setTryDownscale(tryDownscale);
}

void zxing_DecodeHints_setFormats (zxing_DecodeHints * hints, zxing_BarcodeFormat formats)
{
	hints->setFormats((ZXing::BarcodeFormat) formats);
}

void zxing_DecodeHints_setBinarizer (zxing_DecodeHints * hints, zxing_Binarizer binarizer)
{
	hints->setBinarizer((ZXing::Binarizer) binarizer);
}

void zxing_DecodeHints_setEanAddOnSymbol (zxing_DecodeHints * hints, zxing_EanAddOnSymbol eanAddOnSymbol)
{
	hints->setEanAddOnSymbol((ZXing::EanAddOnSymbol) eanAddOnSymbol);
}

void zxing_DecodeHints_setTextMode (zxing_DecodeHints * hints, zxing_TextMode textMode)
{
	hints->setTextMode((ZXing::TextMode) textMode);
}

/*
 * ...
 */

/*
 * ZXing/Result.h
 */

void zxing_Result_delete (zxing_Result * result)
{
	delete result;
}

bool zxing_Result_isValid (const zxing_Result * result)
{
	return result->isValid();
}

zxing_BarcodeFormat zxing_Result_format (const zxing_Result * result)
{
	return (zxing_BarcodeFormat) result->format();
}

/*
 * ...
 */

/*
 * ZXing/ReadBarcode.h
 */

zxing_Result * zxing_ReadBarcode (const zxing_ImageView * iv, const zxing_DecodeHints * hints)
{
	const ZXing::ImageView * cppiv = reinterpret_cast<const ZXing::ImageView *>(iv);
	const ZXing::DecodeHints * cpphints = reinterpret_cast<const ZXing::DecodeHints *>(hints);
	return new ZXing::Result(ZXing::ReadBarcode(*cppiv, *cpphints));
}

zxing_Results * zxing_ReadBarcodes (const zxing_ImageView * iv, const zxing_DecodeHints * hints)
{
	const ZXing::ImageView * cppiv = reinterpret_cast<const ZXing::ImageView *>(iv);
	const ZXing::DecodeHints * cpphints = reinterpret_cast<const ZXing::DecodeHints *>(hints);
	return new ZXing::Results(ZXing::ReadBarcodes(*cppiv, *cpphints));
}

}
