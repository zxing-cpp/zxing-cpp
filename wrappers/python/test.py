import unittest
import zxing
import importlib.util

has_numpy = importlib.util.find_spec('numpy') is not None
has_pil = importlib.util.find_spec('PIL') is not None

BF = zxing.BarcodeFormat


class Test(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxing.barcode_format_from_str('qrcode'), BF.QRCode)
		self.assertEqual(zxing.barcode_formats_from_str('ITF, qrcode'), BF.ITF | BF.QRCode)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_write_read_cycle(self):
		format = BF.QRCode
		text = "I have the best words."
		img = zxing.write_barcode(format, text)

		res = zxing.read_barcode(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		res = zxing.read_barcode2(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		res = zxing.read_barcode2(img, zxing.DecodeHints(formats=format))
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
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
		self.assertEqual(res.position.topLeft.x, 61)

		res = zxing.read_barcode2(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 61)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_failed_read(self):
		import numpy as np
		res = zxing.read_barcode(
			np.zeros((100, 100), np.uint8), formats=BF.EAN8 | BF.Aztec, binarizer=zxing.Binarizer.BoolCast
		)

		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.NONE)
		self.assertEqual(res.text, '')

		hints = zxing.DecodeHints()
		hints.setFormats(BF.EAN8 | BF.Aztec)
		hints.setBinarizer(zxing.Binarizer.BoolCast)
		res = zxing.read_barcode2(np.zeros((100, 10), np.uint8), hints)
		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.NONE)
		self.assertEqual(res.text, '')

		hints = zxing.DecodeHints(formats=BF.EAN8 | BF.Aztec, binarizer=zxing.Binarizer.BoolCast)
		res = zxing.read_barcode2(np.zeros((100, 10), np.uint8), hints)
		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.NONE)
		self.assertEqual(res.text, '')

	@unittest.skipIf(not has_pil, "need numpy for read/write tests")
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
		self.assertEqual(res.position.topLeft.x, 4)

		res = zxing.read_barcode2(img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		rgb_img = img.convert("RGB")
		res = zxing.read_barcode2(rgb_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		rgba_img = img.convert("RGBA")
		res = zxing.read_barcode2(rgba_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		bin_img = img.convert("1")
		res = zxing.read_barcode2(bin_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

		cmyk_img = img.convert("CMYK")
		res = zxing.read_barcode2(cmyk_img)
		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

	def test_read_invalid_type(self):
		self.assertRaisesRegex(
			TypeError, "Unsupported type <class 'str'>. Expect a PIL Image or numpy array", zxing.read_barcode2, "foo"
		)


if __name__ == '__main__':
	unittest.main()
