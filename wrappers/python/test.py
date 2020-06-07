import unittest
import zxing
from numpy import *

BF = zxing.BarcodeFormat

class Test(unittest.TestCase):

	def test_format(self):
		self.assertEqual(zxing.barcode_format_from_str('qrcode'), BF.QR_CODE)
		self.assertEqual(zxing.barcode_formats_from_str('qrcode, ITF'), [BF.QR_CODE, BF.ITF])

	def test_write_read_cycle(self):
		format = BF.QR_CODE
		text = "I have the best words."
		img = zxing.write_barcode(format, text)
		res = zxing.read_barcode(img)

		self.assertTrue(res.valid)
		self.assertEqual(res.format, format)
		self.assertEqual(res.text, text)

	def test_failed_read(self):
		res = zxing.read_barcode(zeros((100, 100), uint8))

		self.assertFalse(res.valid)
		self.assertEqual(res.format, BF.INVALID)
		self.assertEqual(res.text, '')

if __name__ == '__main__':
	unittest.main()
