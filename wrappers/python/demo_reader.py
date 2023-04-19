import sys
import zxingcpp
import cv2
#from PIL import Image

if len(sys.argv) == 1:
	img = cv2.imread('test.png')
#	img = Image.open('test.png')
else:
	img = cv2.imread(sys.argv[1])
#	img = Image.open(sys.argv[1])
results = zxingcpp.read_barcodes(img)
for result in results:
	print('Found barcode:'
		f'\n Text:    "{result.text}"'
		f'\n Format:   {result.format}'
		f'\n Content:  {result.content_type}'
		f'\n Position: {result.position}')
if len(results) == 0:
	print("Could not find any barcode.")
