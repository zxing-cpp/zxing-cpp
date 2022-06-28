import sys, zxingcpp
from PIL import Image

img = Image.open(sys.argv[1])
results = zxingcpp.read_barcodes(img)
for result in results:
	print("Found barcode:\n Text:    '{}'\n Format:   {}\n Content:  {}\n Position: {}"
		.format(result.text, result.format, result.content_type, result.position))
if len(results) == 0:
	print("Could not find any barcode.")
