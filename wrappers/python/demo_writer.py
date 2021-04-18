import sys
import zxingcpp
from PIL import Image

if len(sys.argv) < 3:
	img = zxingcpp.write_barcode(zxingcpp.BarcodeFormat.QRCode, "I have the best words.", width=200, height=200)
else:
	img = zxingcpp.write_barcode(zxingcpp.barcode_format_from_str(sys.argv[1]), sys.argv[2], width=200, height=200)
Image.fromarray(img).save("test.png")
