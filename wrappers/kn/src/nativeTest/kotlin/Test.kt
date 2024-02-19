/*
* Copyright 2024 ISNing
*/
// SPDX-License-Identifier: Apache-2.0

import zxingcpp.*
import kotlin.experimental.ExperimentalNativeApi
import kotlin.test.Test
import kotlin.test.assertContentEquals
import kotlin.test.assertEquals
import kotlin.test.assertNotNull

class BarcodeReaderTest {

	@Test
	@OptIn(ExperimentalNativeApi::class)
	fun `read barcode`() {
		val data = "0000101000101101011110111101011011101010100111011100101000100101110010100000".map {
			if (it == '0') 255.toByte() else 0.toByte()
		}

		val iv = ImageView(data.toByteArray(), data.size, 1, ImageFormat.Lum)
		val br = BarcodeReader().apply {
			binarizer = Binarizer.BoolCast
		}
		val res = br.read(iv).firstOrNull()

		val expected = "96385074"

		assertNotNull(res)
		assert(res.isValid)
		assertEquals(res.format, BarcodeFormat.EAN8)
		assertEquals(res.text, expected)
		assertContentEquals(res.bytes, expected.encodeToByteArray())
		assert(!res.hasECI)
		assertEquals(res.contentType, ContentType.Text)
		assertEquals(res.orientation, 0)
		assertEquals(res.position.topLeft, PointI(4, 0))
		assertEquals(res.lineCount, 1)
	}
}
