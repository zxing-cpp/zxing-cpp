/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#ifndef _ZXING_C_H
#define _ZXING_C_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus

#include "ReaderOptions.h"
#include "ImageView.h"
#include "Result.h"

typedef ZXing::ImageView zxing_ImageView;
typedef ZXing::ReaderOptions zxing_ReaderOptions;
typedef ZXing::Result zxing_Barcode;
typedef ZXing::Results zxing_Barcodes;

extern "C"
{
#else

typedef struct zxing_ImageView zxing_ImageView;
typedef struct zxing_ReaderOptions zxing_ReaderOptions;
typedef struct zxing_Barcode zxing_Barcode;
typedef struct zxing_Barcodes zxing_Barcodes;

#endif

/*
 * ZXing/ImageView.h
 */

typedef enum {
	zxing_ImageFormat_None = 0,
	zxing_ImageFormat_Lum = 0x01000000,
	zxing_ImageFormat_RGB = 0x03000102,
	zxing_ImageFormat_BGR = 0x03020100,
	zxing_ImageFormat_RGBX = 0x04000102,
	zxing_ImageFormat_XRGB = 0x04010203,
	zxing_ImageFormat_BGRX = 0x04020100,
	zxing_ImageFormat_XBGR = 0x04030201,
} zxing_ImageFormat;

zxing_ImageView* zxing_ImageView_new(const uint8_t* data, int width, int height, zxing_ImageFormat format, int rowStride,
									 int pixStride);
zxing_ImageView* zxing_ImageView_new_checked(const uint8_t* data, int size, int width, int height, zxing_ImageFormat format,
											 int rowStride, int pixStride);
void zxing_ImageView_delete(zxing_ImageView* iv);

void zxing_ImageView_crop(zxing_ImageView* iv, int left, int top, int width, int height);
void zxing_ImageView_rotate(zxing_ImageView* iv, int degree);

/*
 * ZXing/BarcodeFormat.h
 */

typedef enum
{
	zxing_BarcodeFormat_None = 0,
	zxing_BarcodeFormat_Aztec = (1 << 0),
	zxing_BarcodeFormat_Codabar = (1 << 1),
	zxing_BarcodeFormat_Code39 = (1 << 2),
	zxing_BarcodeFormat_Code93 = (1 << 3),
	zxing_BarcodeFormat_Code128 = (1 << 4),
	zxing_BarcodeFormat_DataBar = (1 << 5),
	zxing_BarcodeFormat_DataBarExpanded = (1 << 6),
	zxing_BarcodeFormat_DataMatrix = (1 << 7),
	zxing_BarcodeFormat_EAN8 = (1 << 8),
	zxing_BarcodeFormat_EAN13 = (1 << 9),
	zxing_BarcodeFormat_ITF = (1 << 10),
	zxing_BarcodeFormat_MaxiCode = (1 << 11),
	zxing_BarcodeFormat_PDF417 = (1 << 12),
	zxing_BarcodeFormat_QRCode = (1 << 13),
	zxing_BarcodeFormat_UPCA = (1 << 14),
	zxing_BarcodeFormat_UPCE = (1 << 15),
	zxing_BarcodeFormat_MicroQRCode = (1 << 16),
	zxing_BarcodeFormat_RMQRCode = (1 << 17),
	zxing_BarcodeFormat_DXFilmEdge = (1 << 18),

	zxing_BarcodeFormat_LinearCodes = zxing_BarcodeFormat_Codabar | zxing_BarcodeFormat_Code39 | zxing_BarcodeFormat_Code93
									  | zxing_BarcodeFormat_Code128 | zxing_BarcodeFormat_EAN8 | zxing_BarcodeFormat_EAN13
									  | zxing_BarcodeFormat_ITF | zxing_BarcodeFormat_DataBar | zxing_BarcodeFormat_DataBarExpanded
									  | zxing_BarcodeFormat_DXFilmEdge | zxing_BarcodeFormat_UPCA | zxing_BarcodeFormat_UPCE,
	zxing_BarcodeFormat_MatrixCodes = zxing_BarcodeFormat_Aztec | zxing_BarcodeFormat_DataMatrix | zxing_BarcodeFormat_MaxiCode
									  | zxing_BarcodeFormat_PDF417 | zxing_BarcodeFormat_QRCode | zxing_BarcodeFormat_MicroQRCode
									  | zxing_BarcodeFormat_RMQRCode,
	zxing_BarcodeFormat_Any = zxing_BarcodeFormat_LinearCodes | zxing_BarcodeFormat_MatrixCodes,

	zxing_BarcodeFormat_Invalid = 0xFFFFFFFFu /* return value when BarcodeFormatsFromString() throws */
} zxing_BarcodeFormat;

typedef zxing_BarcodeFormat zxing_BarcodeFormats;

zxing_BarcodeFormats zxing_BarcodeFormatsFromString(const char* str);
zxing_BarcodeFormat zxing_BarcodeFormatFromString(const char* str);
char* zxing_BarcodeFormatToString(zxing_BarcodeFormat format);

/*
 * ZXing/ReaderOptions.h
 */

typedef enum
{
	zxing_Binarizer_LocalAverage,
	zxing_Binarizer_GlobalHistogram,
	zxing_Binarizer_FixedThreshold,
	zxing_Binarizer_BoolCast,
} zxing_Binarizer;

typedef enum
{
	zxing_EanAddOnSymbol_Ignore,
	zxing_EanAddOnSymbol_Read,
	zxing_EanAddOnSymbol_Require,
} zxing_EanAddOnSymbol;

typedef enum
{
	zxing_TextMode_Plain,
	zxing_TextMode_ECI,
	zxing_TextMode_HRI,
	zxing_TextMode_Hex,
	zxing_TextMode_Escaped,
} zxing_TextMode;

zxing_ReaderOptions* zxing_ReaderOptions_new();
void zxing_ReaderOptions_delete(zxing_ReaderOptions* opts);

void zxing_ReaderOptions_setTryHarder(zxing_ReaderOptions* opts, bool tryHarder);
void zxing_ReaderOptions_setTryRotate(zxing_ReaderOptions* opts, bool tryRotate);
void zxing_ReaderOptions_setTryInvert(zxing_ReaderOptions* opts, bool tryInvert);
void zxing_ReaderOptions_setTryDownscale(zxing_ReaderOptions* opts, bool tryDownscale);
void zxing_ReaderOptions_setIsPure(zxing_ReaderOptions* opts, bool isPure);
void zxing_ReaderOptions_setReturnErrors(zxing_ReaderOptions* opts, bool returnErrors);
void zxing_ReaderOptions_setFormats(zxing_ReaderOptions* opts, zxing_BarcodeFormats formats);
void zxing_ReaderOptions_setBinarizer(zxing_ReaderOptions* opts, zxing_Binarizer binarizer);
void zxing_ReaderOptions_setEanAddOnSymbol(zxing_ReaderOptions* opts, zxing_EanAddOnSymbol eanAddOnSymbol);
void zxing_ReaderOptions_setTextMode(zxing_ReaderOptions* opts, zxing_TextMode textMode);
void zxing_ReaderOptions_setMinLineCount(zxing_ReaderOptions* opts, int n);
void zxing_ReaderOptions_setMaxNumberOfSymbols(zxing_ReaderOptions* opts, int n);

bool zxing_ReaderOptions_getTryHarder(const zxing_ReaderOptions* opts);
bool zxing_ReaderOptions_getTryRotate(const zxing_ReaderOptions* opts);
bool zxing_ReaderOptions_getTryInvert(const zxing_ReaderOptions* opts);
bool zxing_ReaderOptions_getTryDownscale(const zxing_ReaderOptions* opts);
bool zxing_ReaderOptions_getIsPure(const zxing_ReaderOptions* opts);
bool zxing_ReaderOptions_getReturnErrors(const zxing_ReaderOptions* opts);
zxing_BarcodeFormats zxing_ReaderOptions_getFormats(const zxing_ReaderOptions* opts);
zxing_Binarizer zxing_ReaderOptions_getBinarizer(const zxing_ReaderOptions* opts);
zxing_EanAddOnSymbol zxing_ReaderOptions_getEanAddOnSymbol(const zxing_ReaderOptions* opts);
zxing_TextMode zxing_ReaderOptions_getTextMode(const zxing_ReaderOptions* opts);
int zxing_ReaderOptions_getMinLineCount(const zxing_ReaderOptions* opts);
int zxing_ReaderOptions_getMaxNumberOfSymbols(const zxing_ReaderOptions* opts);

/*
 * ZXing/Result.h
 */

typedef enum
{
	zxing_ContentType_Text,
	zxing_ContentType_Binary,
	zxing_ContentType_Mixed,
	zxing_ContentType_GS1,
	zxing_ContentType_ISO15434,
	zxing_ContentType_UnknownECI
} zxing_ContentType;

char* zxing_ContentTypeToString(zxing_ContentType type);

typedef struct zxing_PointI
{
	int x, y;
} zxing_PointI;

typedef struct zxing_Position
{
	zxing_PointI topLeft, topRight, bottomRight, bottomLeft;
} zxing_Position;

char* zxing_PositionToString(zxing_Position position);

bool zxing_Barcode_isValid(const zxing_Barcode* barcode);
char* zxing_Barcode_errorMsg(const zxing_Barcode* barcode);
zxing_BarcodeFormat zxing_Barcode_format(const zxing_Barcode* barcode);
zxing_ContentType zxing_Barcode_contentType(const zxing_Barcode* barcode);
uint8_t* zxing_Barcode_bytes(const zxing_Barcode* barcode, int* len);
uint8_t* zxing_Barcode_bytesECI(const zxing_Barcode* barcode, int* len);
char* zxing_Barcode_text(const zxing_Barcode* barcode);
char* zxing_Barcode_ecLevel(const zxing_Barcode* barcode);
char* zxing_Barcode_symbologyIdentifier(const zxing_Barcode* barcode);
zxing_Position zxing_Barcode_position(const zxing_Barcode* barcode);
int zxing_Barcode_orientation(const zxing_Barcode* barcode);
bool zxing_Barcode_hasECI(const zxing_Barcode* barcode);
bool zxing_Barcode_isInverted(const zxing_Barcode* barcode);
bool zxing_Barcode_isMirrored(const zxing_Barcode* barcode);
int zxing_Barcode_lineCount(const zxing_Barcode* barcode);

/*
 * ZXing/ReadBarcode.h
 */

/** Note: opts is optional, i.e. it can be NULL, which will imply default settings. */
zxing_Barcode* zxing_ReadBarcode(const zxing_ImageView* iv, const zxing_ReaderOptions* opts);
zxing_Barcodes* zxing_ReadBarcodes(const zxing_ImageView* iv, const zxing_ReaderOptions* opts);

void zxing_Barcode_delete(zxing_Barcode* barcode);
void zxing_Barcodes_delete(zxing_Barcodes* barcodes);

int zxing_Barcodes_size(const zxing_Barcodes* barcodes);
const zxing_Barcode* zxing_Barcodes_at(const zxing_Barcodes* barcodes, int i);
zxing_Barcode* zxing_Barcodes_move(zxing_Barcodes* barcodes, int i);

char* zxing_LastErrorMsg();

void zxing_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* _ZXING_C_H */
