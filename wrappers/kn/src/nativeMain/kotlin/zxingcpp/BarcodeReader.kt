package zxingcpp

import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.zxing_LastErrorMsg
import zxingcpp.cinterop.zxing_ReadBarcodes

@OptIn(ExperimentalForeignApi::class, ExperimentalStdlibApi::class)
class BarcodeReader(val options: ReaderOptions? = null) {
	@Throws(BarcodeReadingException::class)
	fun read(imageView: ImageView): List<Barcode> =
		imageView.cValueWrapped.use {
			zxing_ReadBarcodes(it.cValue, options?.cValue)?.toKObject()
				?: throw BarcodeReadingException(zxing_LastErrorMsg()?.toKStringAndFree())
		}
}

class BarcodeReadingException(message: String?) : Exception("Failed to read barcodes: $message")
