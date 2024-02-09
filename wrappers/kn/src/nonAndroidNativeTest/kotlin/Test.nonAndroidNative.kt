import korlibs.image.format.readBitmap
import korlibs.io.file.std.LocalVfs
import korlibs.io.posix.posixGetcwd
import kotlinx.coroutines.runBlocking
import zxingcpp.ImageView

actual fun loadImage(path: String): ImageView = runBlocking {
    LocalVfs[posixGetcwd()]["src"]["nativeTest"]["resources"][path].readBitmap().asImageView()
}
