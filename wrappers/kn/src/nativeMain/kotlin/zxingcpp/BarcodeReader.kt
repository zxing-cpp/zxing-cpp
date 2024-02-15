package zxingcpp

import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.cinterop.ZXing_Barcodes_delete
import zxingcpp.cinterop.ZXing_LastErrorMsg
import zxingcpp.cinterop.ZXing_ReadBarcodes

@OptIn(ExperimentalForeignApi::class, ExperimentalStdlibApi::class)
class BarcodeReader : ReaderOptions() {
	@Throws(BarcodeReadingException::class)
	fun read(imageView: ImageView): List<Barcode> =
		imageView.use {
			ZXing_ReadBarcodes(it.cValue, cValue)?.let { cValues -> cValues.toKObject().also { ZXing_Barcodes_delete(cValues) } }
				?: throw BarcodeReadingException(ZXing_LastErrorMsg()?.toKStringAndFree())
		}
}

class BarcodeReadingException(message: String?) : Exception("Failed to read barcodes: $message")
