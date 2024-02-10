import kotlinx.cinterop.ExperimentalForeignApi
import zxingcpp.BarcodeReader
import zxingcpp.ImageView
import kotlin.test.Test
import kotlin.test.assertEquals

expect fun loadImage(path: String): ImageView
expect fun readTextFrom(path: String): String

@OptIn(ExperimentalForeignApi::class)
class BarcodeReaderTest {

	private val barcodeReader = BarcodeReader()

	private val samplesDir = "../../test/samples"

	private val samples = mapOf(
		"qrcode-1/1.png" to "qrcode-1/1.txt",
	)

	@Test
	fun `read barcode`() {
		samples.forEach { (path, resultPath) ->
			val imageView = loadImage("$samplesDir/$path")

			val actualBarcode = barcodeReader.read(imageView).firstOrNull()
			val result = readTextFrom("$samplesDir/$resultPath")

			assertEquals(result, actualBarcode?.text)
		}
	}
}
