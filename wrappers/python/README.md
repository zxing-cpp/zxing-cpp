# Python bindings for zxing-cpp

[![PyPI](https://img.shields.io/pypi/v/zxing-cpp.svg)](https://pypi.org/project/zxing-cpp/)

The package uses [scikit-build-core](https://scikit-build-core.readthedocs.io/) as its build backend and [nanobind](https://nanobind.readthedocs.io/) for the Python bindings.

## Installation

```bash
pip install zxing-cpp
```

To build from a checked out / extracted soruce tree, a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a C++20 compiler is required:

```bash
pip install .
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

To get a full list of available parameters for `read_barcodes` and `create_barcode` as well as the properties of the Barcode objects, have a look at the `nanobind` module definition in [this C++ source file](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/python/zxing.cpp).
