# C bindings for zxing-cpp

This is a preview/proposal for a C-API to zxing-cpp. If this turns out to be useful and practical, it will most likely be merged into the library itself so that it will be trivially accessible for everyone. If you have any comments or feedback, please have a look at https://github.com/zxing-cpp/zxing-cpp/discussions/583.

## Installation

Probably the easiest way to play with the C-API is to either just modify the [zxing-c-test.c](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/c/zxing-c-test.c) file or copy the files [zxing-c.h](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/c/zxing-c.h) and [zxing-c.cpp](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/c/zxing-c.cpp) into your own project and link it to the standard zxing-cpp library.

## Usage

The following is close to the most trivial use case scenario that is supported.

```c
#include "zxing-c.h"

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
		printf("Format     : %s\n", zxing_BarcodeFormatToString(zxing_Result_format(result)));

		const char* text = zxing_Result_text(result);
		printf("Text       : %s\n", text);
		free(text);

		zxing_Result_delete(result);
	} else {
		const char* error = zxing_LastErrorMsg();
		if (error) {
			printf("%s\n", error);
			free(error);
		} else {
			printf("No barcode found\n");
		}
	}

	zxing_ImageView_delete(iv);
	zxing_ReaderOptions_delete(opts);

	return 0;
}
```

