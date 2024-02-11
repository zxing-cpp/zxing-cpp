import korlibs.io.file.std.LocalVfs
import korlibs.io.posix.posixGetcwd
import kotlinx.cinterop.*
import kotlinx.coroutines.runBlocking
import stb_image.STBI_grey
import stb_image.stbi_image_free
import stb_image.stbi_load
import zxingcpp.ImageFormat
import zxingcpp.ImageView

@OptIn(ExperimentalForeignApi::class)
actual fun loadImage(path: String): ImageView = memScoped {
	val width = alloc<Int>(0)
	val height = alloc<Int>(0)
	val channels = alloc<Int>(0)
	stbi_load("${posixGetcwd()}/$path", width.ptr, height.ptr, channels.ptr, STBI_grey.toInt())
		?.let { data ->
			ImageView(
				data = data.readBytes(width.value * height.value).toUByteArray(),
				left = 0,
				top = 0,
				width = width.value,
				height = height.value,
				format = ImageFormat.Lum,
				rotation = 0,
				rowStride = 0,
				pixStride = 0
			).also { stbi_image_free(data) }
		} ?: error("Can't load image from $path")
}

actual fun readTextFrom(path: String): String = runBlocking {
	LocalVfs[posixGetcwd()][path].readString()
}
