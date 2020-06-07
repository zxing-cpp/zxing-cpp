import sys, zxing
from PIL import Image

img = Image.open(sys.argv[1])
result = zxing.read_barcode(img)
if result.valid:
	print("Found barcode with value '{}' (format: {})".format(result.text, str(result.format)))
else:
	print("Could not read barcode")
