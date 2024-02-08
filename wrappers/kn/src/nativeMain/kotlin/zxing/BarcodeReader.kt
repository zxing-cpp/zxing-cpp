package zxing

import kotlinx.cinterop.ExperimentalForeignApi
import zxing.cinterop.zxing_ReadBarcode
import zxing.cinterop.zxing_ReadBarcodes

@OptIn(ExperimentalForeignApi::class)
class BarcodeReader {
	fun readBarcode(imageView: ImageView, options: ReaderOptions? = null): Result =
		zxing_ReadBarcode(imageView.toCImageView(), options?.cOptions)?.toKObject()
			?: error("zxing_ReadBarcode returned null, which is an unexpected behaviour")

	fun readBarcodes(imageView: ImageView, options: ReaderOptions? = null): List<Result> =
		zxing_ReadBarcodes(imageView.toCImageView(), options?.cOptions)?.toKObject()
			?: error("zxing_ReadBarcode returned null, which is an unexpected behaviour")
}
