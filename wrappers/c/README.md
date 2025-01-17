# C bindings for zxing-cpp

This is about the C-API of zxing-cpp. If you have any comments or feedback, please have a look at https://github.com/zxing-cpp/zxing-cpp/discussions/583.

## Installation

To enable the C-API, the library needs to be configured with `cmake -DZXING_C_API=ON`.

Probably the easiest way to play with the C-API is to just modify the [ZXingCTest.c](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/c/ZXingCTest.c) file.

## Usage

The following is close to the most trivial use case scenario that is supported.

```c
#include "ZXing/ZXingC.h"

int main(int argc, char** argv)
{
	int width, height;
	unsigned char* data;
	/* load your image data from somewhere. ZXing_ImageFormat_Lum assumes grey scale image data. */

	ZXing_ImageView* iv = ZXing_ImageView_new(data, width, height, ZXing_ImageFormat_Lum, 0, 0);

	ZXing_ReaderOptions* opts = ZXing_ReaderOptions_new();
	/* set ReaderOptions properties, if requried, e.g. */
	ZXing_ReaderOptions_setFormats(ZXing_BarcodeFormat_QRCode | ZXing_BarcodeFromat_EAN13);

	ZXing_Barcodes* barcodes = ZXing_ReadBarcodes(iv, opts);

	ZXing_ImageView_delete(iv);
	ZXing_ReaderOptions_delete(opts);

	if (barcodes) {
		for (int i = 0, n = ZXing_Barcodes_size(barcodes); i < n; ++i) {
			const ZXing_Barcode* barcode = ZXing_Barcodes_at(barcodes, i);

			char* format = ZXing_BarcodeFormatToString(ZXing_Barcode_format(barcode));
			printf("Format     : %s\n", format);
			ZXing_free(format);

			char* text = ZXing_Barcode_text(barcode);
			printf("Text       : %s\n", text);
			ZXing_free(text);
		}
		ZXing_Barcodes_delete(barcodes);
	} else {
		char* error = ZXing_LastErrorMsg();
		fprintf(stderr, "%s\n", error);
		ZXing_free(error);
	}

	return 0;
}
```

