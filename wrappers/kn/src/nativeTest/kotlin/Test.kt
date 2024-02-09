import zxingcpp.*
import kotlin.test.Test
import kotlin.test.assertEquals

expect fun loadImage(path: String): ImageView

class BarcodeReaderTest {

	private val barcodeReader = BarcodeReader()

	val singleExamples = mapOf(
		"qr_100_hello.png" to BarcodeImpl(
			isValid = true,
			errorMsg = "",
			format = BarcodeFormat.QRCode,
			contentType = ContentType.Text,
			text = "Hello zxing-cpp!",
			ecLevel = "L",
			symbologyIdentifier = "]Q1",
			position = Position(
				topLeft = PointI(x = 18, y = 18),
				topRight = PointI(x = 81, y = 18),
				bottomRight = PointI(x = 81, y = 81),
				bottomLeft = PointI(x = 18, y = 81)
			),
			orientation = 0,
			hasECI = false,
			isInverted = false,
			isMirrored = false,
			lineCount = 0
		),
		"qr_512_hello.png" to BarcodeImpl(
			isValid = true,
			errorMsg = "",
			format = BarcodeFormat.QRCode,
			contentType = ContentType.Text,
			text = "Hello zxing-cpp!",
			ecLevel = "L",
			symbologyIdentifier = "]Q1",
			position = Position(
				topLeft = PointI(x = 14, y = 14),
				topRight = PointI(x = 497, y = 14),
				bottomRight = PointI(x = 497, y = 497),
				bottomLeft = PointI(x = 14, y = 497)
			),
			orientation = 0,
			hasECI = false,
			isInverted = false,
			isMirrored = false,
			lineCount = 0
		),
		"qr_100_website.png" to BarcodeImpl(
			isValid = true,
			errorMsg = "",
			format = BarcodeFormat.QRCode,
			contentType = ContentType.Text,
			text = "https://github.com/zxing-cpp",
			ecLevel = "L",
			symbologyIdentifier = "]Q1",
			position = Position(
				topLeft = PointI(x = 12, y = 12),
				topRight = PointI(x = 87, y = 12),
				bottomRight = PointI(x = 87, y = 87),
				bottomLeft = PointI(x = 12, y = 87)
			),
			orientation = 0,
			hasECI = false,
			isInverted = false,
			isMirrored = false,
			lineCount = 0
		),
		"qr_512_website.png" to BarcodeImpl(
			isValid = true,
			errorMsg = "",
			format = BarcodeFormat.QRCode,
			contentType = ContentType.Text,
			text = "https://github.com/zxing-cpp",
			ecLevel = "L",
			symbologyIdentifier = "]Q1",
			position = Position(
				topLeft = PointI(x = 18, y = 18),
				topRight = PointI(x = 493, y = 18),
				bottomRight = PointI(x = 493, y = 493),
				bottomLeft = PointI(x = 18, y = 493)
			),
			orientation = 0,
			hasECI = false,
			isInverted = false,
			isMirrored = false,
			lineCount = 0
		),
		"code128_website.png" to BarcodeImpl(
			isValid = true,
			errorMsg = "",
			format = BarcodeFormat.Code128,
			contentType = ContentType.Text,
			text = "https://github.com/zxing-cpp",
			ecLevel = "",
			symbologyIdentifier = "]C0",
			position = Position(
				topLeft = PointI(x = 5, y = 50),
				topRight = PointI(x = 347, y = 50),
				bottomRight = PointI(x = 347, y = 52),
				bottomLeft = PointI(x = 5, y = 52)
			),
			orientation = 0,
			hasECI = false,
			isInverted = false,
			isMirrored = false,
			lineCount = 2
		),
	)

	val multipleExamples = mapOf(
		"multiple_codes.png" to setOf(
			BarcodeImpl(
				isValid = true,
				errorMsg = "",
				format = BarcodeFormat.Code128,
				contentType = ContentType.Text,
				text = "https://github.com/zxing-cpp",
				ecLevel = "",
				symbologyIdentifier = "]C0",
				position = Position(
					topLeft = PointI(x = 11, y = 87),
					topRight = PointI(x = 353, y = 87),
					bottomRight = PointI(x = 353, y = 106),
					bottomLeft = PointI(x = 11, y = 106)
				),
				orientation = 0,
				hasECI = false,
				isInverted = false,
				isMirrored = false,
				lineCount = 7
			),
			BarcodeImpl(
				isValid = true,
				errorMsg = "",
				format = BarcodeFormat.QRCode,
				contentType = ContentType.Text,
				text = "https://github.com/zxing-cpp",
				ecLevel = "L",
				symbologyIdentifier = "]Q1",
				position = Position(
					topLeft = PointI(x = 174, y = 122),
					topRight = PointI(x = 340, y = 122),
					bottomRight = PointI(x = 340, y = 287),
					bottomLeft = PointI(x = 173, y = 287)
				),
				orientation = 0,
				hasECI = false,
				isInverted = false,
				isMirrored = false,
				lineCount = 0
			),
			BarcodeImpl(
				isValid = true,
				errorMsg = "",
				format = BarcodeFormat.QRCode,
				contentType = ContentType.Text,
				text = "Hello zxing-cpp!",
				ecLevel = "L",
				symbologyIdentifier = "]Q1",
				position = Position(
					topLeft = PointI(x = 12, y = 126),
					topRight = PointI(x = 157, y = 126),
					bottomRight = PointI(x = 157, y = 271),
					bottomLeft = PointI(x = 12, y = 271)
				),
				orientation = 0,
				hasECI = false,
				isInverted = false,
				isMirrored = false,
				lineCount = 0
			)
		)
	)

	@Test
	fun `reads single barcode`() {
		singleExamples.forEach { (path, expectedBarcode) ->
			val imageView = loadImage(path)
			val options = ReaderOptions()

			val actualBarcode = barcodeReader.readBarcode(imageView, options)

			assertEquals(expectedBarcode, actualBarcode)
		}
	}

	@Test
	fun `reads multiple barcodes`() {
		multipleExamples.forEach { (path, expectedBarcodes) ->
			val imageView = loadImage(path)
			val options = ReaderOptions()

			val actualBarcodes = barcodeReader.readBarcodes(imageView, options).toSet()

			println(actualBarcodes)

			assertEquals(expectedBarcodes, actualBarcodes)
		}
	}
}
