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

#include "DecodeHints.h"
#include "ImageView.h"
#include "Result.h"

typedef ZXing::ImageView zxing_ImageView;
typedef ZXing::DecodeHints zxing_DecodeHints;
typedef ZXing::Result zxing_Result;
typedef ZXing::Results zxing_Results;

extern "C"
{
#else

typedef struct zxing_ImageView zxing_ImageView;
typedef struct zxing_DecodeHints zxing_DecodeHints;
typedef struct zxing_Result zxing_Result;
typedef struct zxing_Results zxing_Results;

#endif

/*
 * ZXing/ImageView.h
 */

typedef enum
{
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
void zxing_ImageView_delete(zxing_ImageView* iv);

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

	zxing_BarcodeFormat_LinearCodes = zxing_BarcodeFormat_Codabar | zxing_BarcodeFormat_Code39 | zxing_BarcodeFormat_Code93
									  | zxing_BarcodeFormat_Code128 | zxing_BarcodeFormat_EAN8 | zxing_BarcodeFormat_EAN13
									  | zxing_BarcodeFormat_ITF | zxing_BarcodeFormat_DataBar | zxing_BarcodeFormat_DataBarExpanded
									  | zxing_BarcodeFormat_UPCA | zxing_BarcodeFormat_UPCE,
	zxing_BarcodeFormat_MatrixCodes = zxing_BarcodeFormat_Aztec | zxing_BarcodeFormat_DataMatrix | zxing_BarcodeFormat_MaxiCode
									  | zxing_BarcodeFormat_PDF417 | zxing_BarcodeFormat_QRCode | zxing_BarcodeFormat_MicroQRCode,
	zxing_BarcodeFormat_Any = zxing_BarcodeFormat_LinearCodes | zxing_BarcodeFormat_MatrixCodes,

	zxing_BarcodeFormat_Invalid = -1 /* return value when BarcodeFormatsFromString() throws */
} zxing_BarcodeFormat;

typedef zxing_BarcodeFormat zxing_BarcodeFormats;

zxing_BarcodeFormats zxing_BarcodeFormatsFromString(const char* str);
zxing_BarcodeFormat zxing_BarcodeFormatFromString(const char* str);
char* zxing_BarcodeFormatToString(zxing_BarcodeFormat format);

/*
 * ZXing/DecodeHints.h
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

zxing_DecodeHints* zxing_DecodeHints_new();
void zxing_DecodeHints_delete(zxing_DecodeHints* hints);

void zxing_DecodeHints_setTryHarder(zxing_DecodeHints* hints, bool tryHarder);
void zxing_DecodeHints_setTryRotate(zxing_DecodeHints* hints, bool tryRotate);
void zxing_DecodeHints_setTryInvert(zxing_DecodeHints* hints, bool tryInvert);
void zxing_DecodeHints_setTryDownscale(zxing_DecodeHints* hints, bool tryDownscale);
void zxing_DecodeHints_setIsPure(zxing_DecodeHints* hints, bool isPure);
void zxing_DecodeHints_setReturnErrors(zxing_DecodeHints* hints, bool returnErrors);
void zxing_DecodeHints_setFormats(zxing_DecodeHints* hints, zxing_BarcodeFormats formats);
void zxing_DecodeHints_setBinarizer(zxing_DecodeHints* hints, zxing_Binarizer binarizer);
void zxing_DecodeHints_setEanAddOnSymbol(zxing_DecodeHints* hints, zxing_EanAddOnSymbol eanAddOnSymbol);
void zxing_DecodeHints_setTextMode(zxing_DecodeHints* hints, zxing_TextMode textMode);

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

bool zxing_Result_isValid(const zxing_Result* result);
char* zxing_Result_errorMsg(const zxing_Result* result);
zxing_BarcodeFormat zxing_Result_format(const zxing_Result* result);
zxing_ContentType zxing_Result_contentType(const zxing_Result* result);
uint8_t* zxing_Result_bytes(const zxing_Result* result, int* len);
char* zxing_Result_text(const zxing_Result* result);
char* zxing_Result_ecLevel(const zxing_Result* result);
char* zxing_Result_symbologyIdentifier(const zxing_Result* result);
int zxing_Result_orientation(const zxing_Result* result);
bool zxing_Result_isInverted(const zxing_Result* result);
bool zxing_Result_isMirrored(const zxing_Result* result);

/*
 * ZXing/ReadBarcode.h
 */

zxing_Result* zxing_ReadBarcode(const zxing_ImageView* iv, const zxing_DecodeHints* hints);
zxing_Results* zxing_ReadBarcodes(const zxing_ImageView* iv, const zxing_DecodeHints* hints);

void zxing_Result_delete(zxing_Result* result);
void zxing_Results_delete(zxing_Results* results);

int zxing_Results_size(const zxing_Results* results);
const zxing_Result* zxing_Results_at(const zxing_Results* results, int i);

#ifdef __cplusplus
}
#endif

#endif /* _ZXING_C_H */
