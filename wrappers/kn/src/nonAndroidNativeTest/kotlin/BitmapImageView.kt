import korlibs.image.bitmap.Bitmap
import korlibs.image.bitmap.Bitmap16
import korlibs.image.bitmap.Bitmap32
import korlibs.image.bitmap.BitmapIndexed
import korlibs.image.color.*
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.readBytes
import kotlinx.cinterop.usePinned
import zxingcpp.ImageFormat
import zxingcpp.ImageView

fun Bitmap.toImageView() = when (this) {
	is Bitmap32 -> toImageView()
	is Bitmap16 -> toImageView()
	is BitmapIndexed -> toImageView()
	else -> toBMP32().toImageView()
}

fun BitmapIndexed.toImageView() = ImageView(
	data = data.toUByteArray(),
	left = 0,
	top = 0,
	width = width,
	height = height,
	rotation = 0,
	format = ImageFormat.RGBX
)

@OptIn(ExperimentalForeignApi::class)
fun Bitmap16.toImageView(): ImageView {
	val imgFormat = when (format) {
		YUVA -> ImageFormat.Lum
		RGB -> ImageFormat.RGB
		BGR -> ImageFormat.BGR
		RGBA -> ImageFormat.RGBX
		ARGB -> ImageFormat.XRGB
		BGRA -> ImageFormat.BGRX
		else -> error("Unsupported format: $format")
	}
	val data = when (format) {
		YUVA -> data.filterIndexed { index, _ -> index % 4 == 0 }.toShortArray().usePinned {
			it.addressOf(0).readBytes(area * 4).toUByteArray()
		}

		else -> data.usePinned {
			it.addressOf(0).readBytes(area * bpp).toUByteArray()
		}
	}
	return ImageView(
		data = data,
		left = 0,
		top = 0,
		width = width,
		height = height,
		rotation = 0,
		format = imgFormat
	)
}

fun Bitmap32.toImageView() = ImageView(
	data = extractBytes(RGB).asUByteArray(),
	left = 0,
	top = 0,
	width = width,
	height = height,
	rotation = 0,
	format = ImageFormat.RGB
)
