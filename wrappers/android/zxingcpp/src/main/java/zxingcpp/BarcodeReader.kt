/*
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp

import android.graphics.Bitmap
import android.graphics.ImageFormat
import android.graphics.Point
import android.graphics.Rect
import android.os.Build
import androidx.camera.core.ImageProxy
import java.nio.ByteBuffer

public class BarcodeReader(public var options: Options = Options()) {
	private val supportedYUVFormats: List<Int> =
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			listOf(ImageFormat.YUV_420_888, ImageFormat.YUV_422_888, ImageFormat.YUV_444_888)
		} else {
			listOf(ImageFormat.YUV_420_888)
		}

	init {
		System.loadLibrary("zxingcpp_android")
	}

	// Enumerates barcode formats known to this package.
	// Note that this has to be kept synchronized with native (C++/JNI) side.
	public enum class Format {
		NONE, AZTEC, CODABAR, CODE_39, CODE_93, CODE_128, DATA_BAR, DATA_BAR_EXPANDED, DATA_BAR_LIMITED,
		DATA_MATRIX, DX_FILM_EDGE, EAN_8, EAN_13, ITF, MAXICODE, PDF_417, QR_CODE, MICRO_QR_CODE, RMQR_CODE, UPC_A, UPC_E
	}

	public enum class ContentType {
		TEXT, BINARY, MIXED, GS1, ISO15434, UNKNOWN_ECI
	}

	public enum class Binarizer {
		LOCAL_AVERAGE, GLOBAL_HISTOGRAM, FIXED_THRESHOLD, BOOL_CAST
	}

	public enum class EanAddOnSymbol {
		IGNORE, READ, REQUIRE
	}

	public enum class TextMode {
		PLAIN, ECI, HRI, HEX, ESCAPED
	}

	public enum class ErrorType {
		FORMAT, CHECKSUM, UNSUPPORTED
	}

	public data class Options(
		var formats: Set<Format> = setOf(),
		var tryHarder: Boolean = false,
		var tryRotate: Boolean = false,
		var tryInvert: Boolean = false,
		var tryDownscale: Boolean = false,
		var isPure: Boolean = false,
		var binarizer: Binarizer = Binarizer.LOCAL_AVERAGE,
		var downscaleFactor: Int = 3,
		var downscaleThreshold: Int = 500,
		var minLineCount: Int = 2,
		var maxNumberOfSymbols: Int = 0xff,
		var tryCode39ExtendedMode: Boolean = true,
		@Deprecated("See https://github.com/zxing-cpp/zxing-cpp/discussions/704")
		var validateCode39CheckSum: Boolean = false,
		@Deprecated("See https://github.com/zxing-cpp/zxing-cpp/discussions/704")
		var validateITFCheckSum: Boolean = false,
		@Deprecated("See https://github.com/zxing-cpp/zxing-cpp/discussions/704")
		var returnCodabarStartEnd: Boolean = false,
		var returnErrors: Boolean = false,
		var eanAddOnSymbol: EanAddOnSymbol = EanAddOnSymbol.IGNORE,
		var textMode: TextMode = TextMode.HRI,
	)

	public data class Error(
		val type: ErrorType,
		val message: String
	)

	public data class Position(
		val topLeft: Point,
		val topRight: Point,
		val bottomRight: Point,
		val bottomLeft: Point,
		val orientation: Double
	)

	public data class Result(
		val format: Format,
		val bytes: ByteArray?,
		val text: String?,
		val contentType: ContentType,
		val position: Position,
		val orientation: Int,
		val ecLevel: String?,
		val symbologyIdentifier: String?,
		val sequenceSize: Int,
		val sequenceIndex: Int,
		val sequenceId: String?,
		val readerInit: Boolean,
		val lineCount: Int,
		val error: Error?,
	)

	public val lastReadTime : Int = 0 // runtime of last read call in ms (for debugging purposes only)

	public fun read(image: ImageProxy): List<Result> {
		check(image.format in supportedYUVFormats) {
			"Invalid image format: ${image.format}. Must be one of: $supportedYUVFormats"
		}

		return readYBuffer(
			image.planes[0].buffer,
			image.planes[0].rowStride,
			image.cropRect.left,
			image.cropRect.top,
			image.cropRect.width(),
			image.cropRect.height(),
			image.imageInfo.rotationDegrees,
			options
		)
	}

	public fun read(
		bitmap: Bitmap, cropRect: Rect = Rect(), rotation: Int = 0
	): List<Result> {
		return readBitmap(
			bitmap, cropRect.left, cropRect.top, cropRect.width(), cropRect.height(), rotation, options
		)
	}

	private external fun readYBuffer(
		yBuffer: ByteBuffer, rowStride: Int, left: Int, top: Int, width: Int, height: Int, rotation: Int, options: Options
	): List<Result>

	private external fun readBitmap(
		bitmap: Bitmap, left: Int, top: Int, width: Int, height: Int, rotation: Int, options: Options
	): List<Result>
}
