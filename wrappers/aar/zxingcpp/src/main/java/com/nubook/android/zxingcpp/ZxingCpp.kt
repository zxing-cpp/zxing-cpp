package com.nubook.android.zxingcpp

import android.graphics.Bitmap
import android.graphics.Rect
import java.nio.ByteBuffer

object ZxingCpp {
	// Enumerates barcode formats known to this package.
	// Note that this has to be kept synchronized with native (C++/JNI) side.
	enum class Format {
		NONE,
		AZTEC,
		CODABAR,
		CODE_39,
		CODE_93,
		CODE_128,
		DATA_BAR,
		DATA_BAR_EXPANDED,
		DATA_MATRIX,
		EAN_8,
		EAN_13,
		ITF,
		MAXICODE,
		PDF_417,
		QR_CODE,
		UPC_A,
		UPC_E,
	}

	data class Result(
		val format: String,
		val text: String,
		val position: Rect,
		val orientation: Int,
		val rawBytes: ByteArray,
		val numBits: Int,
		val ecLevel: String,
		val symbologyIdentifier: String,
		val sequenceSize: Int,
		val sequenceIndex: Int,
		val sequenceId: String,
		val readerInit: Boolean,
		val lineCount: Int,
		val gtin: GTIN?
	)

	data class GTIN(
		val country: String,
		val addOn: String,
		val price: String,
		val issueNumber: String
	)

	fun readYBuffer(
		yBuffer: ByteBuffer,
		rowStride: Int,
		cropRect: Rect = Rect(),
		rotation: Int = 0,
		formats: Set<Format> = setOf(),
		tryHarder: Boolean = false,
		tryRotate: Boolean = false
	): Result? = readYBuffer(
		yBuffer,
		rowStride,
		cropRect.left, cropRect.top,
		cropRect.width(), cropRect.height(),
		rotation,
		formats.joinToString(),
		tryHarder,
		tryRotate
	)

	private external fun readYBuffer(
		yBuffer: ByteBuffer,
		rowStride: Int,
		left: Int, top: Int,
		width: Int, height: Int,
		rotation: Int,
		formats: String,
		tryHarder: Boolean,
		tryRotate: Boolean
	): Result?

	fun readByteArray(
		yuvData: ByteArray,
		rowStride: Int,
		cropRect: Rect = Rect(),
		rotation: Int = 0,
		formats: Set<Format> = setOf(),
		tryHarder: Boolean = false,
		tryRotate: Boolean = false
	): Result? = readByteArray(
		yuvData,
		rowStride,
		cropRect.left, cropRect.top,
		cropRect.width(), cropRect.height(),
		rotation,
		formats.joinToString(),
		tryHarder,
		tryRotate
	)

	private external fun readByteArray(
		yuvData: ByteArray,
		rowStride: Int,
		left: Int, top: Int,
		width: Int, height: Int,
		rotation: Int,
		formats: String,
		tryHarder: Boolean,
		tryRotate: Boolean
	): Result?

	fun readBitmap(
		bitmap: Bitmap,
		cropRect: Rect = Rect(),
		rotation: Int = 0,
		formats: Set<Format> = setOf(),
		tryHarder: Boolean = false,
		tryRotate: Boolean = false
	): Result? = readBitmap(
		bitmap,
		cropRect.left, cropRect.top,
		cropRect.width(), cropRect.height(),
		rotation,
		formats.joinToString(),
		tryHarder,
		tryRotate
	)

	private external fun readBitmap(
		bitmap: Bitmap,
		left: Int, top: Int,
		width: Int, height: Int,
		rotation: Int,
		formats: String,
		tryHarder: Boolean,
		tryRotate: Boolean
	): Result?

	data class BitMatrix(
		val width: Int,
		val height: Int,
		val data: ByteArray
	) {
		fun get(x: Int, y: Int) = data[y * width + x] != 0.toByte()
	}

	fun encodeAsBitmap(
		text: String,
		format: Format,
		width: Int,
		height: Int,
		margin: Int = 0,
		eccLevel: Int = -1,
		encoding: String = "UTF8",
		setColor: Int = 0xff000000.toInt(),
		unsetColor: Int = 0xffffffff.toInt()
	): Bitmap? {
		val bitMatrix = encode(
			text, format.toString(),
			width, height,
			margin, eccLevel, encoding
		)
		val w = bitMatrix.width
		val h = bitMatrix.height
		val pixels = IntArray(w * h)
		var offset = 0
		for (y in 0 until h) {
			for (x in 0 until w) {
				pixels[offset + x] = if (bitMatrix.get(x, y)) {
					setColor
				} else {
					unsetColor
				}
			}
			offset += w
		}
		val bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
		bitmap.setPixels(pixels, 0, w, 0, 0, w, h)
		return bitmap
	}

	fun encodeAsSvg(
		text: String,
		format: Format,
		margin: Int = 0,
		eccLevel: Int = -1,
		encoding: String = "UTF8"
	): String {
		val bitMatrix = encode(
			text, format.toString(),
			0, 0,
			margin, eccLevel, encoding
		)
		val sb = StringBuilder()
		val w = bitMatrix.width
		var h = bitMatrix.height
		val moduleHeight = if (h == 1) w / 2 else 1
		for (y in 0 until h) {
			for (x in 0 until w) {
				if (bitMatrix.get(x, y)) {
					sb.append(" M${x},${y}h1v${moduleHeight}h-1z")
				}
			}
		}
		h *= moduleHeight
		return """<svg width="$w" height="$h"
viewBox="0 0 $w $h"
xmlns="http://www.w3.org/2000/svg">
<path d="$sb"/>
</svg>
"""
	}

	fun encodeAsText(
		text: String,
		format: Format,
		margin: Int = 0,
		eccLevel: Int = -1,
		encoding: String = "UTF8"
	): String {
		val bitMatrix = encode(
			text, format.toString(),
			0, 0,
			margin, eccLevel, encoding
		)
		val w = bitMatrix.width
		val h = bitMatrix.height
		val sb = StringBuilder()
		for (y in 0 until h) {
			for (x in 0 until w) {
				sb.append(if (bitMatrix.get(x, y)) "█" else " ")
			}
			sb.append("\n")
		}
		return sb.toString()
	}

	private external fun encode(
		text: String,
		format: String,
		width: Int,
		height: Int,
		margin: Int = 0,
		eccLevel: Int = -1,
		encoding: String = "UTF8"
	): BitMatrix

	init {
		System.loadLibrary("zxing")
	}
}
