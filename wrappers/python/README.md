# Python bindings for zxing-cpp

[![PyPI](https://img.shields.io/pypi/v/zxing-cpp.svg)](https://pypi.org/project/zxing-cpp/)


## Installation

```bash
pip install zxing-cpp
```
or

```bash
python setup.py install
```

In case there is no pre-build wheel available for your platform or python version or if you use `setup.py` directly, a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a c++20 compiler is required. To build from source, you can call:

```bash
pip install zxing-cpp --no-binary zxing-cpp
```


## Usage

### Reading barcodes

```python
import cv2, zxingcpp

img = cv2.imread('test.png')
barcodes = zxingcpp.read_barcodes(img)
for barcode in barcodes:
	print('Found barcode:'
		f'\n Text:    "{barcode.text}"'
		f'\n Format:   {barcode.format}'
		f'\n Content:  {barcode.content_type}'
		f'\n Position: {barcode.position}')
if len(barcodes) == 0:
	print("Could not find any barcode.")
```

### Writing barcodes

```python
import zxingcpp
from PIL import Image

barcode = zxingcpp.create_barcode('This is a test', zxingcpp.BarcodeFormat.QRCode, ec_level = "50%")

img = barcode.to_image(scale = 5)
Image.fromarray(img).save("test.png")

svg = barcode.to_svg(add_quiet_zones = False)
with open("test.svg", "w") as svg_file:
	svg_file.write(svg)
```

To get a full list of available parameters for `read_barcodes` and `create_barcode` as well as the properties of the Barcode objects, have a look at the `PYBIND11_MODULE` definition in [this c++ source file](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/python/zxing.cpp).
