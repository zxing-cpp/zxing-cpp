import korlibs.image.format.readBitmap
import korlibs.io.file.std.LocalVfs
import korlibs.io.posix.posixGetcwd
import kotlinx.coroutines.runBlocking
import zxingcpp.ImageView

actual fun loadImage(path: String): ImageView = runBlocking {
	LocalVfs[posixGetcwd()][path].readBitmap().asImageView()
}

actual fun readTextFrom(path: String): String = runBlocking {
	LocalVfs[posixGetcwd()][path].readString()
}
