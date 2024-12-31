/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingC.h"

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR // prevent dependency on -lm
#define STBI_NO_HDR
#include <stb_image.h>

int usage(char* pname)
{
	fprintf(stderr, "ZXingCTest %s, usage: %s FILE [FORMATS]\n", ZXing_Version(), pname);
	return 1;
}

bool parse_args(int argc, char** argv, char** filename, ZXing_BarcodeFormats* formats)
{
	if (argc < 2)
		return false;
	*filename = argv[1];
	if (argc >= 3) {
		*formats = ZXing_BarcodeFormatsFromString(argv[2]);
		if (*formats == ZXing_BarcodeFormat_Invalid) {
			fprintf(stderr, "%s\n", ZXing_LastErrorMsg());
			return false;
		}
	}
	return true;
}

void printF(const char* fmt, char* text)
{
	if (!text)
		return;
	if (*text)
		printf(fmt, text);
	ZXing_free(text);
}

#define CHECK(GOOD) \
	if (!(GOOD)) { \
		char* error = ZXing_LastErrorMsg(); \
		fprintf(stderr, "CHECK(%s) failed: %s\n", #GOOD, error); \
		ZXing_free(error); \
		return 2; \
	}

int main(int argc, char** argv)
{
	int ret = 0;
	char* filename = NULL;
	ZXing_BarcodeFormats formats = ZXing_BarcodeFormat_None;

	if (!parse_args(argc, argv, &filename, &formats))
		return usage(argv[0]);

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* data = stbi_load(filename, &width, &height, &channels, STBI_grey);

	ZXing_ImageView* iv = NULL;
	ZXing_Image* img = NULL;

	if (data) {
		iv = ZXing_ImageView_new(data, width, height, ZXing_ImageFormat_Lum, 0, 0);
		CHECK(iv)
	} else {
		fprintf(stderr, "Could not read image '%s'\n", filename);
#if defined(ZXING_EXPERIMENTAL_API) && defined(ZXING_WRITERS)
		if (formats == ZXing_BarcodeFormat_Invalid)
			return 2;
		fprintf(stderr, "Using '%s' as text input to create barcode\n", filename);
		ZXing_CreatorOptions* cOpts = ZXing_CreatorOptions_new(formats);
		CHECK(cOpts)
		ZXing_Barcode* barcode = ZXing_CreateBarcodeFromText(filename, 0, cOpts);
		CHECK(barcode)
		img = ZXing_WriteBarcodeToImage(barcode, NULL);
		CHECK(img)
		ZXing_CreatorOptions_delete(cOpts);
		ZXing_Barcode_delete(barcode);
#else
		return 2;
#endif
	}

	ZXing_ReaderOptions* opts = ZXing_ReaderOptions_new();
	ZXing_ReaderOptions_setTextMode(opts, ZXing_TextMode_HRI);
	ZXing_ReaderOptions_setEanAddOnSymbol(opts, ZXing_EanAddOnSymbol_Ignore);
	ZXing_ReaderOptions_setFormats(opts, formats);
	ZXing_ReaderOptions_setReturnErrors(opts, true);

	ZXing_Barcodes* barcodes = ZXing_ReadBarcodes(iv ? iv : (ZXing_ImageView*)img, opts);
	CHECK(barcodes)

	ZXing_ImageView_delete(iv);
	ZXing_Image_delete(img);
	ZXing_ReaderOptions_delete(opts);
	stbi_image_free(data);

	for (int i = 0, n = ZXing_Barcodes_size(barcodes); i < n; ++i) {
		const ZXing_Barcode* barcode = ZXing_Barcodes_at(barcodes, i);

		printF("Text       : %s\n", ZXing_Barcode_text(barcode));
		printF("BytesECI   : %s\n", (char*)ZXing_Barcode_bytesECI(barcode, NULL));
		printF("Format     : %s\n", ZXing_BarcodeFormatToString(ZXing_Barcode_format(barcode)));
		printF("Content    : %s\n", ZXing_ContentTypeToString(ZXing_Barcode_contentType(barcode)));
		printF("Identifier : %s\n", ZXing_Barcode_symbologyIdentifier(barcode));
		printf("HasECI     : %d\n", ZXing_Barcode_hasECI(barcode));
		printF("EC Level   : %s\n", ZXing_Barcode_ecLevel(barcode));
		printF("Error      : %s\n", ZXing_Barcode_errorMsg(barcode));
		printF("Position   : %s\n", ZXing_PositionToString(ZXing_Barcode_position(barcode)));
		printf("Rotation   : %d\n", ZXing_Barcode_orientation(barcode));
		printf("IsMirrored : %d\n", ZXing_Barcode_isMirrored(barcode));
		printf("IsInverted : %d\n", ZXing_Barcode_isInverted(barcode));

		if (i < n-1)
			printf("\n");
	}

	if (ZXing_Barcodes_size(barcodes) == 0)
		printf("No barcode found\n");

	ZXing_Barcodes_delete(barcodes);

	return ret;
}
