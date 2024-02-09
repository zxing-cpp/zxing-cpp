import korlibs.image.bitmap.Bitmap
import kotlinx.cinterop.*
import zxingcpp.ImageFormat
import zxingcpp.ImageView

class BitmapImageView(
    val bitmap: Bitmap,
    override val left: Int = 0,
    override val top: Int = 0,
    override val width: Int = bitmap.width,
    override val height: Int = bitmap.height,
    override val rotation: Int = 0,
) : ImageView() {
    @OptIn(ExperimentalForeignApi::class)
    override val data: UByteArray
        get() = bitmap.toBMP32IfRequired().ints.usePinned {
            it.addressOf(0).reinterpret<UByteVar>().readBytes(it.get().size * 4).toUByteArray()
        }
    override val format: ImageFormat
        get() = ImageFormat.RGBX // Actually, it's RGBA, but ZXing doesn't support it, so we use RGBX to avoid the alpha channel
}

fun Bitmap.asImageView() = BitmapImageView(this)
