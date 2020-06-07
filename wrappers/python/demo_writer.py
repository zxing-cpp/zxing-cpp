import sys
import zxing
from PIL import Image

if len(sys.argv) < 3:
	img = zxing.write_barcode(zxing.BarcodeFormat.QR_CODE, "I have the best words.", width=200, height=200)
else:
	img = zxing.write_barcode(zxing.barcode_format_from_str(sys.argv[1]), sys.argv[2], width=200, height=200)
Image.fromarray(img).save("test.png")
