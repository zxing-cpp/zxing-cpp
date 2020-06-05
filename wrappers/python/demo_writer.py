import zxing
from PIL import Image

img = zxing.write_barcode(zxing.BarcodeFormat.QR_CODE, "I have the best words.", width=200, height=200)
Image.fromarray(img).save("test.png")
