#include "zxing.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int usage(char* pname)
{
	fprintf(stderr, "Usage: %s FILE FORMATS\n", pname);
	return 1;
}

bool parse_args(int argc, char** argv, char** filename, char** formats)
{
	if (argc != 3)
		return false;
	*filename = argv[1];
	*formats = argv[2];
	return true;
}

int main(int argc, char** argv)
{
	char* filename = NULL;
	char* formats = NULL;

	if (!parse_args(argc, argv, &filename, &formats))
		return usage(argv[0]);

	int x = 0;
	int y = 0;
	int channels_in_file = 0;
	stbi_uc* data = stbi_load(filename, &x, &y, &channels_in_file, STBI_rgb);
	if (!data)
		return 2;

	zxing_DecodeHints* hints = zxing_DecodeHints_new();
	zxing_DecodeHints_setTextMode(hints, zxing_TextMode_Plain);
	zxing_DecodeHints_setEanAddOnSymbol(hints, zxing_EanAddOnSymbol_Ignore);
	zxing_DecodeHints_setFormats(hints, zxing_BarcodeFormatFromString(formats));

	zxing_ImageView* iv = zxing_ImageView_new(data, x, y, zxing_ImageFormat_RGB, 0, 0);

	zxing_Result* result = zxing_ReadBarcode(iv, hints);
	fprintf(stderr, "Result is %s\n", zxing_Result_isValid(result) ? "valid" : "invalid");

	zxing_Result_delete(result);
	zxing_ImageView_delete(iv);
	zxing_DecodeHints_delete(hints);
	free(data);

	return 0;
}
