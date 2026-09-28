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

### Image formats

Pillow images can be passed directly to `read_barcodes`. NumPy arrays must have
dtype `uint8`. Grayscale arrays can have shape `(height, width)` or
`(height, width, 1)`. Three-channel arrays are interpreted as BGR, matching
OpenCV's default `imread` output.

An RGBA Pillow image is accepted directly, but a four-channel NumPy array raises
`ValueError: Unsupported number of channels for buffer: 4`. If you need to pass
an image loaded by Pillow as a NumPy array, convert it to grayscale first:

```python
import numpy as np
import zxingcpp
from PIL import Image

with Image.open('test.png') as img:
	barcodes = zxingcpp.read_barcodes(np.asarray(img.convert('L')))
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
