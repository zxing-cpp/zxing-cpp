# C bindings for zxing-cpp

This is a preview/proposal for a C-API to zxing-cpp. If you have any comments or feedback, please have a look at https://github.com/zxing-cpp/zxing-cpp/discussions/583.

## Installation

It is currently included in the default build to be trivially accessible for everyone.

Probably the easiest way to play with the C-API is to just modify the [zxing-c-test.c](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/c/zxing-c-test.c) file.

## Usage

The following is close to the most trivial use case scenario that is supported.

```c
#include "ZXing/zxing-c.h"

int main(int argc, char** argv)
{
	int width, height;
	unsigned char* data;
	/* load your image data from somewhere. ImageFormat_Lum assumes grey scale image data. */

	zxing_ImageView* iv = zxing_ImageView_new(data, width, height, zxing_ImageFormat_Lum, 0, 0);

	zxing_ReaderOptions* opts = zxing_ReaderOptions_new();
	/* set ReaderOptions properties, if requried */

	zxing_Result* result = zxing_ReadBarcode(iv, opts);

	if (result) {
		const char* format = zxing_BarcodeFormatToString(zxing_Result_format(result));
		printf("Format     : %s\n", format);
		zxing_free(format);

		const char* text = zxing_Result_text(result);
		printf("Text       : %s\n", text);
		zxing_free(text);

		zxing_Result_delete(result);
	} else {
		const char* error = zxing_LastErrorMsg();
		if (error) {
			printf("%s\n", error);
		} else {
			printf("No barcode found\n");
		}
		zxing_free(error);
	}

	zxing_ImageView_delete(iv);
	zxing_ReaderOptions_delete(opts);

	return 0;
}
```

