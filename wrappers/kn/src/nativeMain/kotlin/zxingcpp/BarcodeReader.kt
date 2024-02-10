package zxingcpp

import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.zxing_ReadBarcodes

@OptIn(ExperimentalForeignApi::class, ExperimentalStdlibApi::class)
class BarcodeReader(val options: ReaderOptions? = null) {
	fun read(imageView: ImageView): List<Barcode> =
		imageView.cValueWrapped.use {
			zxing_ReadBarcodes(it.cValue, options?.cValue)?.toKObject()
				?: error("zxing_ReadBarcodes returned null, which is an unexpected behaviour")
		}
}
