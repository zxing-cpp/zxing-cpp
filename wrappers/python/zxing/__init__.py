from ._zxing import (
	Binarizer, DecodeHints as _DecodeHints, BarcodeFormat, barcode_format_from_str, barcode_formats_from_str,
	read_barcode, read_barcode2 as _read_barcode2, write_barcode
)


__all__ = [
	"BarcodeFormat", "Binarizer", "DecodeHints",
	"barcode_format_from_str", "barcode_formats_from_str", "read_barcode", "read_barcode2", "write_barcode"
]


class DecodeHints(_DecodeHints):

	def __init__(self, **kwargs):
		super().__init__()
		self.update(**kwargs)

	def update(self, **kwargs):
		"""
		Update hints from given (snake case) keyword arguments

		:param kwargs: dictionary of hints to update
		:return: the current DecodeHints object
		:rtype: zxing.DecodeHints
		"""
		for attr_name, value in kwargs.items():
			setter_name = "".join(x.title() for x in attr_name.split("_"))
			getattr(self, f"set{setter_name}")(value)
		return self

	def setFormats(self, value):
		"""
		Override setter to allow BarcodeFormat instead of BarcodeFormats

		:param value: the barcode(s) format(s)
		:type value: zxing.BarcodeFormat|zxing.BarcodeFormats
		:return: the current DecodeHints object
		:rtype: zxing.DecodeHints
		"""
		if isinstance(value, BarcodeFormat):
			value = value | BarcodeFormat.NONE
		return super().setFormats(value)


def read_barcode2(image, hints=DecodeHints()):
	"""
	Read barcode on given image, which can be a PIL Image as well as an opencv image

	:param image: the image to read
	:type image: PIL.Image.Image|numpy.ndarray
	:param zxing.DecodeHints hints: decoding options
	:return: the decoding result
	:rtype: zxing.Result
	"""
	# Test for PIL.Image without requiring that PIL is installed.
	if 'PIL.' in str(type(image)):
		if image.mode != "L":
			image = image.convert("L")
		import numpy as np  # Import here because numpy is not required until reading barcodes
		image = np.array(image)
	return _read_barcode2(image, hints)
