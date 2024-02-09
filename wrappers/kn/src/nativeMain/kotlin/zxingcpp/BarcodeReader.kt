package zxingcpp

import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.zxing_ReadBarcode
import zxingcpp.cinterop.zxing_ReadBarcodes

@OptIn(ExperimentalForeignApi::class, ExperimentalStdlibApi::class)
class BarcodeReader {
	fun readBarcode(imageView: ImageView, options: ReaderOptions? = null): Barcode? =
		imageView.cValueWrapped.use {
			zxing_ReadBarcode(it.cValue, options?.cValue)?.toKObject()
		}

	fun readBarcodes(imageView: ImageView, options: ReaderOptions? = null): List<Barcode> =
		imageView.cValueWrapped.use {
			zxing_ReadBarcodes(it.cValue, options?.cValue)?.toKObject()
				?: error("zxing_ReadBarcodes returned null, which is an unexpected behaviour")
		}
}
