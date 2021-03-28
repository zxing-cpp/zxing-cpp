import importlib.util
import unittest

import numpy as np
import zxing

has_pil = importlib.util.find_spec('PIL') is not None

BF = zxing.BarcodeFormat


class Test(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxing.barcode_format_from_str('qrcode'), BF.QRCode)
		self.assertEqual(zxing.barcode_formats_from_str('ITF, qrcode'), BF.ITF | BF.QRCode)

	def test_write_read_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxing.write_barcode(format, text)

		res = zxing.read_barcode(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

		res = zxing.read_barcode(img, formats=format)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

	def test_write_read_oned_cycle(self):
		format = BF.Code128
		text = "I have the best words."
		height = 80
		width = 400
		img = zxing.write_barcode(format, text, width=width, height=height)
		self.assertEqual(img.shape[0], height)
		self.assertEqual(img.shape[1], width)

		res = zxing.read_barcode(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 61)

	def test_failed_read(self):
		res = zxing.read_barcode(
			np.zeros((100, 100), np.uint8), formats=BF.EAN8 | BF.Aztec, binarizer=zxing.Binarizer.BoolCast
		)

		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.NONE)
		self.assertEqual(res.text, '')

	@unittest.skipIf(not has_pil, "need PIL for read/write tests")
	def test_write_read_cycle_pil(self):
		from PIL import Image
		format = BF.QRCode
		text = "I have the best words."
		img = zxing.write_barcode(format, text)
		img = Image.fromarray(img, "L")

		res = zxing.read_barcode(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

		rgb_img = img.convert("RGB")
		res = zxing.read_barcode(rgb_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

		rgba_img = img.convert("RGBA")
		res = zxing.read_barcode(rgba_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

		bin_img = img.convert("1")
		res = zxing.read_barcode(bin_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

		cmyk_img = img.convert("CMYK")
		res = zxing.read_barcode(cmyk_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.top_left.x, 4)

	def test_read_invalid_type(self):
		self.assertRaisesRegex(
			TypeError, "Unsupported type <class 'str'>. Expect a PIL Image or numpy array", zxing.read_barcode, "foo"
		)

	def test_read_invalid_numpy_array_channels(self):
		self.assertRaisesRegex(
			TypeError, "Unsupported number of channels for numpy array: 4", zxing.read_barcode,
			np.zeros((100, 100, 4), np.uint8)
		)


if __name__ == '__main__':
	unittest.main()
