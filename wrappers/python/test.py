import importlib.util
import unittest

import zxingcpp

has_numpy = importlib.util.find_spec('numpy') is not None
has_pil = importlib.util.find_spec('PIL') is not None

BF = zxingcpp.BarcodeFormat


class TestFormat(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxingcpp.barcode_format_from_str('qrcode'), BF.QRCode)
		self.assertEqual(zxingcpp.barcode_formats_from_str('ITF, qrcode'), BF.ITF | BF.QRCode)


@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
class TestReadWrite(unittest.TestCase):

	def check_res(self, res, format, text):
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.bytes, bytes(text, 'utf-8'))
		self.assertEqual(res.orientation, 0)

	def test_write_read_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.write_barcode(format, text)

		res = zxingcpp.read_barcode(img)
		self.check_res(res, format, text)
		self.assertEqual(res.symbology_identifier, "]Q1")
		self.assertEqual(res.position.top_left.x, 4)

		res = zxingcpp.read_barcode(img, formats=format)
		self.check_res(res, format, text)

	def test_write_read_oned_cycle(self):
		format = BF.Code128
		text = "I have the best words."
		height = 80
		width = 400
		img = zxingcpp.write_barcode(format, text, width=width, height=height)
		self.assertEqual(img.shape[0], height)
		self.assertEqual(img.shape[1], width)

		res = zxingcpp.read_barcode(img)
		self.check_res(res, format, text)
		self.assertEqual(res.position.top_left.x, 61)

	def test_write_read_multi_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.write_barcode(format, text)

		res = zxingcpp.read_barcodes(img)[0]
		self.check_res(res, format, text)

	def test_failed_read(self):
		import numpy as np
		res = zxingcpp.read_barcode(
			np.zeros((100, 100), np.uint8), formats=BF.EAN8 | BF.Aztec, binarizer=zxingcpp.Binarizer.BoolCast
		)

		self.assertEqual(res, None)

	@unittest.skipIf(not has_pil, "need PIL for read/write tests")
	def test_write_read_cycle_pil(self):
		from PIL import Image
		format = BF.QRCode
		text = "I have the best words."
		img = zxingcpp.write_barcode(format, text)
		img = Image.fromarray(img, "L")

		self.check_res(zxingcpp.read_barcode(img), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("RGB")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("RGBA")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("1")), format, text)
		self.check_res(zxingcpp.read_barcode(img.convert("CMYK")), format, text)

	def test_read_invalid_type(self):
		self.assertRaisesRegex(
			TypeError, "Unsupported type <class 'str'>. Expect a PIL Image or numpy array", zxingcpp.read_barcode, "foo"
		)

	def test_read_invalid_numpy_array_channels(self):
		import numpy as np
		self.assertRaisesRegex(
			TypeError, "Unsupported number of channels for numpy array: 4", zxingcpp.read_barcode,
			np.zeros((100, 100, 4), np.uint8)
		)


if __name__ == '__main__':
	unittest.main()
