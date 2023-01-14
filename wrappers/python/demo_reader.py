import sys
import zxingcpp
from cv2 import imread

img = imread(sys.argv[1])
results = zxingcpp.read_barcodes(img)
for result in results:
    print(f'Found barcode:\n Text:    "{result.text}"\n Format:   {result.format}\n Content:  {result.content_type}\n Position: {result.position}')
if len(results) == 0:
    print("Could not find any barcode.")
