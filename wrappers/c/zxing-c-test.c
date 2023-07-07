/*
* Copyright 2023 siiky
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "zxing-c.h"

#define STB_IMAGE_IMPLEMENTATION
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
			fprintf(stderr, "Invalid barcode formats string '%s'\n", argv[2]);
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
	free(text);
}

int main(int argc, char** argv)
{
	char* filename = NULL;
	zxing_BarcodeFormats formats = zxing_BarcodeFormat_None;

	if (!parse_args(argc, argv, &filename, &formats))
		return usage(argv[0]);

	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* data = stbi_load(filename, &width, &height, &channels, STBI_grey);
	if (!data)
		return 2;

	zxing_DecodeHints* hints = zxing_DecodeHints_new();
	zxing_DecodeHints_setTextMode(hints, zxing_TextMode_HRI);
	zxing_DecodeHints_setEanAddOnSymbol(hints, zxing_EanAddOnSymbol_Ignore);
	zxing_DecodeHints_setFormats(hints, formats);
	zxing_DecodeHints_setReturnErrors(hints, true);

	zxing_ImageView* iv = zxing_ImageView_new(data, width, height, zxing_ImageFormat_Lum, 0, 0);

	zxing_Results* results = zxing_ReadBarcodes(iv, hints);

	if (results) {
		for (int i = 0, n = zxing_Results_size(results); i < n; ++i) {
			const zxing_Result* result = zxing_Results_at(results, i);

			printF("Text       : %s\n", zxing_Result_text(result));
			printF("Format     : %s\n", zxing_BarcodeFormatToString(zxing_Result_format(result)));
			printF("Content    : %s\n", zxing_ContentTypeToString(zxing_Result_contentType(result)));
			printF("Identifier : %s\n", zxing_Result_symbologyIdentifier(result));
			printF("EC Level   : %s\n", zxing_Result_ecLevel(result));
			printF("Error      : %s\n", zxing_Result_errorMsg(result));
			printf("Rotation   : %d\n", zxing_Result_orientation(result));

			if (i < n-1)
				printf("\n");
		}

		zxing_Results_delete(results);
	} else {
		printf("No barcode found\n");
	}

	zxing_ImageView_delete(iv);
	zxing_DecodeHints_delete(hints);
	stbi_image_free(data);

	return 0;
}
