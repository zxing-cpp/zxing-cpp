import sys
import zxingcpp
from PIL import Image

if len(sys.argv) < 3:
	print(f"Usage: {sys.argv[0]} <format> <content>\n\nExample: {sys.argv[0]} QRCode 'I have the best words.'\n")
	print("Available formats:\n" + "\n".join(f" - {fmt}" for fmt in zxingcpp.barcode_formats_list(zxingcpp.BarcodeFormat.AllCreatable)))
	sys.exit()

format, content = zxingcpp.barcode_format_from_str(sys.argv[1]), sys.argv[2]

# new writer API
barcode = zxingcpp.create_barcode(content, format, ec_level = "50%")

img = barcode.to_image(scale = 5)
Image.fromarray(img).save("test.png")

svg = barcode.to_svg(add_hrt = True)
with open("test.svg", "w") as svg_file:
	svg_file.write(svg)

# old and deprecated writer API
# img = zxingcpp.write_barcode(format, content, width=200, height=200)
# Image.fromarray(img).save("test.png")
