# Python bindings for zxing-cpp

[![Build + Deploy](https://github.com/zxing-cpp/zxing-cpp/actions/workflows/python-build.yml/badge.svg)](https://github.com/zxing-cpp/zxing-cpp/actions/workflows/python-build.yml)
[![PyPI](https://img.shields.io/pypi/v/zxing-cpp.svg)](https://pypi.org/project/zxing-cpp/)

## Installation

```bash
pip install --upgrade zxing-cpp
```

or

```bash
python setup.py install
```

Note: To install via `setup.py` (or via `pip install` in case there is no pre-build wheel available for your platform or python version), you need a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a C++ compiler.

## Usage

For the examples, you need to install Pillow (for PIL) and/or CV2 with:

```bash
pip install --upgrade Pillow opencv-python
```

An example for writing a code to a PNG file with the help of PIL is:

```python
import sys
import zxingcpp
from PIL import Image

if len(sys.argv) < 3:
	img = zxingcpp.write_barcode(zxingcpp.BarcodeFormat.QRCode,
		'I have the best words.', width=200, height=200)
else:
	img = zxingcpp.zxingcwrite_barcode(zxingcpp.barcode_format_from_str(sys.argv[1]),
		sys.argv[2], width=200, height=200)
Image.fromarray(img).save('test.png')
```

An example for reading a code from a PNG file with the help of OpenCV (or PIL) is:

```python
import sys
import zxingcpp
import cv2
#import PIL

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
```
