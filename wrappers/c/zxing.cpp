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

zxing_ImageView zxing_ImageView_new (const uint8_t * data, int width, int height, zxing_ImageFormat format, int rowStride, int pixStride)
{
	ZXing::ImageFormat cppformat = (ZXing::ImageFormat) format;
	ZXing::ImageView * cppiv = new ZXing::ImageView(data, width, height, cppformat, rowStride, pixStride);
	return (zxing_ImageView) cppiv;
}

void zxing_ImageView_free (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	delete cppiv;
	//cppiv->~ImageView();
}

int zxing_ImageView_width (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	return cppiv->width();
}

int zxing_ImageView_height (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	return cppiv->height();
}

int zxing_ImageView_pixStride (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	return cppiv->pixStride();
}

int zxing_ImageView_rowStride (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	return cppiv->rowStride();
}

zxing_ImageFormat zxing_ImageView_format (zxing_ImageView iv)
{
	ZXing::ImageView * cppiv = (ZXing::ImageView *) iv;
	ZXing::ImageFormat cppformat = cppiv->format();
	return (zxing_ImageFormat) cppformat;
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

zxing_DecodeHints zxing_DecodeHints_new ()
{
	ZXing::DecodeHints * cpphints = new ZXing::DecodeHints();
	return (zxing_DecodeHints) cpphints;
}

void zxing_DecodeHints_free (zxing_DecodeHints hints)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	delete cpphints;
}

void zxing_DecodeHints_setTryHarder (zxing_DecodeHints hints, bool tryHarder)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	cpphints->setTryHarder(tryHarder);
}

void zxing_DecodeHints_setTryDownscale (zxing_DecodeHints hints, bool tryDownscale)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	cpphints->setTryDownscale(tryDownscale);
}

void zxing_DecodeHints_setFormats (zxing_DecodeHints hints, zxing_BarcodeFormat formats)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	ZXing::BarcodeFormat cppformats = (ZXing::BarcodeFormat) formats;
	cpphints->setFormats(cppformats);
}

void zxing_DecodeHints_setBinarizer (zxing_DecodeHints hints, zxing_Binarizer binarizer)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	ZXing::Binarizer cppbinarizer = (ZXing::Binarizer) binarizer;
	cpphints->setBinarizer(cppbinarizer);
}

void zxing_DecodeHints_setEanAddOnSymbol (zxing_DecodeHints hints, zxing_EanAddOnSymbol eanAddOnSymbol)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	ZXing::EanAddOnSymbol cppeanAddOnSymbol = (ZXing::EanAddOnSymbol) eanAddOnSymbol;
	cpphints->setEanAddOnSymbol(cppeanAddOnSymbol);
}

void zxing_DecodeHints_setTextMode (zxing_DecodeHints hints, zxing_TextMode textMode)
{
	ZXing::DecodeHints * cpphints = (ZXing::DecodeHints *) hints;
	ZXing::TextMode cpptextMode = (ZXing::TextMode) textMode;
	cpphints->setTextMode(cpptextMode);
}

/*
 * ...
 */

/*
 * ZXing/Result.h
 */

void zxing_Result_free (zxing_Result result)
{
    ZXing::Result * cppresult = (ZXing::Result *) result;
    delete cppresult;
}

bool zxing_Result_isValid (zxing_Result result)
{
	ZXing::Result * cppresult = (ZXing::Result *) result;
	return cppresult->isValid();
}

zxing_BarcodeFormat zxing_Result_format (zxing_Result result)
{
	ZXing::Result * cppresult = (ZXing::Result *) result;
	return (zxing_BarcodeFormat) cppresult->format();
}

/*
 * ...
 */

/*
 * ZXing/ReadBarcode.h
 */

zxing_Result zxing_ReadBarcode (const zxing_ImageView iv, const zxing_DecodeHints hints)
{
	const ZXing::ImageView & cppiv = (const ZXing::ImageView &) iv;
	// TODO
	//zxing.cpp: In function ‘void* zxing_ReadBarcode(zxing_ImageView, zxing_DecodeHints)’:
	//zxing.cpp:157:76: warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
	//  157 |         const ZXing::DecodeHints & cpphints = (const ZXing::DecodeHints &) hints;
	const ZXing::DecodeHints & cpphints = (const ZXing::DecodeHints &) hints;
	ZXing::Result cppresult = ZXing::ReadBarcode(cppiv, cpphints);
	ZXing::Result * cppresultptr = new ZXing::Result(std::move(cppresult));
	return (zxing_Result) cppresultptr;
}

zxing_Results zxing_ReadBarcodes (const zxing_ImageView iv, const zxing_DecodeHints hints)
{
	const ZXing::ImageView & cppiv = (const ZXing::ImageView &) iv;
	// TODO
	//zxing.cpp: In function ‘void* zxing_ReadBarcodes(zxing_ImageView, zxing_DecodeHints)’:
	//zxing.cpp:173:76: warning: dereferencing type-punned pointer will break strict-aliasing rules [-Wstrict-aliasing]
	//  173 |         const ZXing::DecodeHints & cpphints = (const ZXing::DecodeHints &) hints;
	const ZXing::DecodeHints & cpphints = (const ZXing::DecodeHints &) hints;
	ZXing::Results cppresults = ZXing::ReadBarcodes(cppiv, cpphints);
	ZXing::Results * cppresultsptr = new ZXing::Results(std::move(cppresults));
	return (zxing_Results) cppresultsptr;
	return NULL;
}

}
