import unittest
import zxing
import importlib.util

has_numpy = importlib.util.find_spec('numpy') is not None

BF = zxing.BarcodeFormat

class Test(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxing.barcode_format_from_str('qrcode'), BF.QR_CODE)
		self.assertEqual(zxing.barcode_formats_from_str('ITF, qrcode'), BF.ITF | BF.QR_CODE)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_write_read_cycle(self):
		format = BF.QR_CODE
		text = "I have the best words."
		img = zxing.write_barcode(format, text)
		res = zxing.read_barcode(img)

		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)
		self.assertEqual(res.orientation, 0)
		self.assertEqual(res.position.topLeft.x, 4)

	@unittest.skipIf(not has_numpy, "need numpy for read/write tests")
	def test_failed_read(self):
		import numpy as np
		res = zxing.read_barcode(np.zeros((100, 100), np.uint8), formats = BF.EAN_8 | BF.AZTEC, binarizer = zxing.BoolCast)

		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.NONE)
		self.assertEqual(res.text, '')

if __name__ == '__main__':
	unittest.main()
