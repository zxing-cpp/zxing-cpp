import sys, zxingcpp
from PIL import Image

img = Image.open(sys.argv[1])
results = zxingcpp.read_barcodes(img)
for result in results:
	print('Found barcode:'
		f'\n Text:    "{result.text}"'
		f'\n Format:   {result.format}'
		f'\n Content:  {result.content_type}'
		f'\n Position: {result.position}')
if len(results) == 0:
	print("Could not find any barcode.")
