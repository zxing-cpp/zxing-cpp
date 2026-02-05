import importlib.util
import unittest
import math
import platform

import zxingcpp # pyright: ignore[reportMissingImports]

has_numpy = importlib.util.find_spec('numpy') is not None
has_pil = importlib.util.find_spec('PIL') is not None
has_cv2 = importlib.util.find_spec('cv2') is not None

BF = zxingcpp.BarcodeFormat
CT = zxingcpp.ContentType

class TestFormat(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxingcpp.barcode_format_from_str('qrcode'), BF.QRCode)
		self.assertEqual(zxingcpp.barcode_formats_from_str('ITF, qrcode'), [BF.ITF, BF.QRCode])
		self.assertEqual(BF.Code128 | BF.EAN13, [BF.Code128, BF.EAN13])
		self.assertEqual(zxingcpp.BarcodeFormats(BF.EAN13), [BF.EAN13])
		self.assertEqual(zxingcpp.BarcodeFormats(BF.EAN13), BF.EAN13)
		self.assertEqual(str(BF.QRCode), "QR Code")
		self.assertEqual(BF.EAN13.symbology, BF.EANUPC)

class TestReadWrite(unittest.TestCase):

	def check_res(self, res, format, text):
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.bytes, bytes(text, 'utf-8'))
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.content_type, CT.Text)

	def test_create_write_read_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.create_barcode(text, format, ec_level="L", version=2).to_image()

		res = zxingcpp.read_barcode(img, format)
		self.check_res(res, format, text)
		self.assertEqual(res.ec_level, "L")
		self.assertEqual(res.symbology_identifier, "]Q1")
		self.assertEqual(res.extra["Version"], '2')

	def test_create_from_str(self):
		format = BF.Code128
		text = "I have the best words."
		res = zxingcpp.create_barcode(text, format)

		# res = zxingcpp.read_barcode(img)
		self.check_res(res, format, text)
		# self.assertEqual(res.position.top_left.x, 61)

	def test_create_from_bytes(self):
		format = BF.DataMatrix
		text = b"\1\2\3\4"
		res = zxingcpp.create_barcode(text, format)

		self.assertTrue(res.valid)
		self.assertEqual(res.bytes, text)
		self.assertEqual(res.content_type, CT.Binary)

	def test_create_write_read_multi_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.create_barcode(text, format).to_image(scale=5)

		res = zxingcpp.read_barcodes(img)[0]
		self.check_res(res, format, text)

	@unittest.skipIf(not hasattr(zxingcpp, 'write_barcode'), "skipping test for deprecated write_barcode API")
	def test_write_read_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.write_barcode(format, text, ec_level = 8)

		res = zxingcpp.read_barcode(img)
		self.check_res(res, format, text)
		self.assertEqual(res.symbology_identifier, "]Q1")
		self.assertEqual(res.ec_level, "H")
		# self.assertEqual(res.position.top_left.x, 4)

		res = zxingcpp.read_barcode(img, formats=[format])
		self.check_res(res, format, text)

	@unittest.skipIf(not hasattr(zxingcpp, 'write_barcode'), "skipping test for deprecated write_barcode API")
	def test_write_read_bytes_cycle(self):
		format = BF.QRCode
		text = b"\1\2\3\4"
		img = zxingcpp.write_barcode(format, text)

		res = zxingcpp.read_barcode(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.bytes, text)
		self.assertEqual(res.content_type, CT.Binary)

	@staticmethod
	def zeroes(shape):
		return memoryview(b"0" * math.prod(shape)).cast("B", shape=shape)

	def test_failed_read_buffer(self):
		res = zxingcpp.read_barcode(
			self.zeroes((100, 100)), formats=[BF.EAN8, BF.Aztec], binarizer=zxingcpp.Binarizer.BoolCast
		)

		self.assertEqual(res, None)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_failed_read_numpy(self):
		import numpy as np # pyright: ignore
		res = zxingcpp.read_barcode(
			np.zeros((100, 100), np.uint8), formats=[BF.EAN8, BF.Aztec], binarizer=zxingcpp.Binarizer.BoolCast
		)

		self.assertEqual(res, None)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_write_read_cycle_numpy(self):
		import numpy as np # pyright: ignore
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.create_barcode(text, format).to_image()
		img = np.array(img)

		self.check_res(zxingcpp.read_barcode(img), format, text)
		self.check_res(zxingcpp.read_barcode(img[4:40,4:40]), format, text)

	@unittest.skipIf(not has_pil, "need PIL for read/write tests")
	def test_write_read_cycle_pil(self):
		from PIL import Image # pyright: ignore
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.create_barcode(text, format).to_image(scale=5)
		img = Image.fromarray(img, "L")

		self.check_res(zxingcpp.read_barcode(img), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("RGB")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("RGBA")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("1")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("CMYK")), format, text)

	@unittest.skipIf(not has_cv2, "need cv2 for read/write tests")
	def test_write_read_cycle_cv2(self):
		import cv2, numpy # pyright: ignore
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.create_barcode(text, format).to_image()
		img = cv2.cvtColor(numpy.array(img), cv2.COLOR_GRAY2BGR )

		self.check_res(zxingcpp.read_barcode(img), format, text)
		self.check_res(zxingcpp.read_barcode(img[4:40,4:40,:]), format, text)

	def test_read_invalid_type(self):
		self.assertRaisesRegex(
			TypeError, "Invalid input: <class 'str'> does not support the buffer protocol.", zxingcpp.read_barcode, "foo"
		)

	def test_read_invalid_numpy_array_channels_buffer(self):
		self.assertRaisesRegex(
			ValueError, "Unsupported number of channels for buffer: 4", zxingcpp.read_barcode,
			self.zeroes((100, 100, 4))
		)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_read_invalid_numpy_array_channels_numpy(self):
		import numpy as np # pyright: ignore
		self.assertRaisesRegex(
			ValueError, "Unsupported number of channels for buffer: 4", zxingcpp.read_barcode,
			np.zeros((100, 100, 4), np.uint8)
		)

	def test_image_buffer_protocol(self):
		"""Test that Image objects support the buffer protocol"""
		format = BF.QRCode
		text = "Buffer protocol test"
		img = zxingcpp.create_barcode(text, format).to_image()

		# Test memoryview
		mv = memoryview(img)
		self.assertEqual(mv.ndim, 2)
		self.assertEqual(mv.format, 'B')
		self.assertEqual(mv.readonly, True)
		self.assertEqual(mv.shape, (img.shape[0], img.shape[1]))

		# Test that we can read the barcode back using memoryview
		res = zxingcpp.read_barcode(mv, format)
		self.check_res(res, format, text)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_image_buffer_protocol_numpy(self):
		"""Test that Image objects can be converted to numpy arrays via buffer protocol"""
		import numpy as np # pyright: ignore
		format = BF.QRCode
		text = "Numpy buffer test"
		img = zxingcpp.create_barcode(text, format).to_image()

		# Convert Image to numpy array via buffer protocol
		arr = np.array(img, copy=False)
		self.assertEqual(arr.ndim, 2)
		self.assertEqual(arr.dtype, np.uint8)
		self.assertEqual(arr.shape, (img.shape[0], img.shape[1]))

		# Test that we can read the barcode back
		res = zxingcpp.read_barcode(arr, format)
		self.check_res(res, format, text)

	def test_imageview_buffer_protocol(self):
		"""Test that ImageView objects support the buffer protocol"""
		format = BF.QRCode
		text = "ImageView buffer test"
		img = zxingcpp.create_barcode(text, format).to_image()

		# Create ImageView from memoryview
		mv = memoryview(img)
		iv = zxingcpp.ImageView(mv, img.shape[1], img.shape[0], zxingcpp.ImageFormat.Lum)

		# Test memoryview of ImageView
		mv2 = memoryview(iv)
		self.assertEqual(mv2.ndim, 2)
		self.assertEqual(mv2.format, 'B')
		self.assertEqual(mv2.readonly, True)

		# Test that we can read the barcode back using memoryview
		res = zxingcpp.read_barcode(mv2, format)
		self.check_res(res, format, text)


if __name__ == '__main__':
	unittest.main()
