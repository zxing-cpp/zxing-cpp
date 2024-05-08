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
		assertEquals(BarcodeFormat.EAN8, res.format)
		assertEquals(expected, res.text)
		assertContentEquals(expected.encodeToByteArray(), res.bytes)
		assert(!res.hasECI)
		assertEquals(ContentType.Text, res.contentType)
		assertEquals(0, res.orientation)
		assertEquals(PointI(4, 0), res.position.topLeft)
		assertEquals(1, res.lineCount)
	}

	@Test
	@OptIn(ExperimentalNativeApi::class, ExperimentalWriterApi::class)
	fun `create write and read barcode with text`() {
		val text = "I have the best words."
		val barcode = Barcode(text, BarcodeFormat.DataMatrix)
		val image = barcode.toImage()

		val res = BarcodeReader.read(image.toImageView()).firstOrNull()

		assertNotNull(res)
		assert(res.isValid)
		assertEquals(BarcodeFormat.DataMatrix, res.format)
		assertEquals(text, res.text)
		assertContentEquals(text.encodeToByteArray(), res.bytes)
		assert(!res.hasECI)
		assertEquals(ContentType.Text, res.contentType)
		assertEquals(0, res.orientation)
		assertEquals(PointI(1, 1), res.position.topLeft)
		assertEquals(0, res.lineCount)
	}

	@Test
	@OptIn(ExperimentalNativeApi::class, ExperimentalWriterApi::class)
	fun `create write and read barcode with bytes`() {
		val text = "I have the best words."
		val barcode = Barcode(text.encodeToByteArray(), BarcodeFormat.DataMatrix)
		val image = barcode.toImage()

		val res = BarcodeReader.read(image.toImageView()).firstOrNull()

		assertNotNull(res)
		assert(res.isValid)
		assertEquals(BarcodeFormat.DataMatrix, res.format)
		assertEquals(text, res.text)
		assertContentEquals(text.encodeToByteArray(), res.bytes)
		assert(res.hasECI)
		assertEquals(ContentType.Binary, res.contentType)
		assertEquals(0, res.orientation)
		assertEquals(PointI(1, 1), res.position.topLeft)
		assertEquals(0, res.lineCount)
	}
}
