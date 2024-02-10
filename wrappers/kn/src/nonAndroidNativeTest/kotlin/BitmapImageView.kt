import korlibs.image.bitmap.Bitmap
import kotlinx.cinterop.*
import zxingcpp.ImageFormat
import zxingcpp.ImageView

@OptIn(ExperimentalForeignApi::class)
fun Bitmap.asImageView() = ImageView(
	data = toBMP32IfRequired().ints.usePinned {
		it.addressOf(0).reinterpret<UByteVar>().readBytes(it.get().size * 4).toUByteArray()
	},
	left = 0,
	top = 0,
	width = width,
	height = height,
	rotation = 0,
	format = ImageFormat.RGBX // Actually, it's RGBA, but ZXing doesn't support it, so we use RGBX to avoid the alpha channel
)
