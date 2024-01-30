/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "zxing-c.h"

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR // prevent dependency on -lm
#define STBI_NO_HDR
#include <stb_image.h>

int usage(char* pname)
{
	fprintf(stderr, "Usage: %s FILE [FORMATS]\n", pname);
	return 1;
}

bool parse_args(int argc, char** argv, char** filename, zxing_BarcodeFormats* formats)
{
	if (argc < 2)
		return false;
	*filename = argv[1];
	if (argc >= 3) {
		*formats = zxing_BarcodeFormatsFromString(argv[2]);
		if (*formats == zxing_BarcodeFormat_Invalid) {
			fprintf(stderr, "%s\n", zxing_LastErrorMsg());
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
	zxing_free(text);
}

int main(int argc, char** argv)
{
	int ret = 0;
	char* filename = NULL;
	zxing_BarcodeFormats formats = zxing_BarcodeFormat_None;

	if (!parse_args(argc, argv, &filename, &formats))
		return usage(argv[0]);

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* data = stbi_load(filename, &width, &height, &channels, STBI_grey);
	if (!data) {
		fprintf(stderr, "Could not read image '%s'\n", filename);
		return 2;
	}

	zxing_ReaderOptions* opts = zxing_ReaderOptions_new();
	zxing_ReaderOptions_setTextMode(opts, zxing_TextMode_HRI);
	zxing_ReaderOptions_setEanAddOnSymbol(opts, zxing_EanAddOnSymbol_Ignore);
	zxing_ReaderOptions_setFormats(opts, formats);
	zxing_ReaderOptions_setReturnErrors(opts, true);

	zxing_ImageView* iv = zxing_ImageView_new(data, width, height, zxing_ImageFormat_Lum, 0, 0);

	zxing_Barcodes* barcodes = zxing_ReadBarcodes(iv, opts);

	zxing_ImageView_delete(iv);
	zxing_ReaderOptions_delete(opts);
	stbi_image_free(data);

	if (barcodes) {
		for (int i = 0, n = zxing_Barcodes_size(barcodes); i < n; ++i) {
			const zxing_Barcode* barcode = zxing_Barcodes_at(barcodes, i);

			printF("Text       : %s\n", zxing_Barcode_text(barcode));
			printF("Format     : %s\n", zxing_BarcodeFormatToString(zxing_Barcode_format(barcode)));
			printF("Content    : %s\n", zxing_ContentTypeToString(zxing_Barcode_contentType(barcode)));
			printF("Identifier : %s\n", zxing_Barcode_symbologyIdentifier(barcode));
			printF("EC Level   : %s\n", zxing_Barcode_ecLevel(barcode));
			printF("Error      : %s\n", zxing_Barcode_errorMsg(barcode));
			printF("Position   : %s\n", zxing_PositionToString(zxing_Barcode_position(barcode)));
			printf("Rotation   : %d\n", zxing_Barcode_orientation(barcode));

			if (i < n-1)
				printf("\n");
		}

		if (zxing_Barcodes_size(barcodes) == 0)
			printf("No barcode found\n");

		zxing_Barcodes_delete(barcodes);
	} else {
		char* error = zxing_LastErrorMsg();
		fprintf(stderr, "%s\n", error);
		zxing_free(error);
		ret = 2;
	}

	return ret;
}
