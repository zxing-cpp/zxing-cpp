import sys
import zxingcpp
from PIL import Image

if len(sys.argv) < 3:
	format, content = zxingcpp.BarcodeFormat.QRCode, "I have the best words."
else:
	format, content = zxingcpp.barcode_format_from_str(sys.argv[1]), sys.argv[2]

# old writer API
img = zxingcpp.write_barcode(format, content, width=200, height=200)
Image.fromarray(img).save("test.png")

# new/experimental writer API
# barcode = zxingcpp.create_barcode(content, format, ec_level = "50%")

# img = barcode.to_image(size_hint = 500)
# Image.fromarray(img).save("test.png")

# svg = barcode.to_svg(with_hrt = True)
# with open("test.svg", "w") as svg_file:
# 	svg_file.write(svg)
