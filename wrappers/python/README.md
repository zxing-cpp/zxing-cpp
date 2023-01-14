# Python bindings for zxing-cpp

[![Build + Deploy](https://github.com/zxing-cpp/zxing-cpp/actions/workflows/python-build.yml/badge.svg)](https://github.com/zxing-cpp/zxing-cpp/actions/workflows/python-build.yml)
[![PyPI](https://img.shields.io/pypi/v/zxing-cpp.svg)](https://pypi.org/project/zxing-cpp/)

## Installation

```bash
pip install zxing-cpp
```
or

```bash
python setup.py install
```

Note: To install via `setup.py`, you need a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a c++ compiler.

## Usage

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

An example for reading a code from a PNG file with the help of OpenCV is:

```python
import sys
import zxingcpp
from cv2 import imread

img = imread(sys.argv[1])
results = zxingcpp.read_barcodes(img)
for result in results:
    print(f'Found barcode:\n Text:    "{result.text}"\n Format:   {result.format}\n Content:  {result.content_type}\n Position: {result.position}')
if len(results) == 0:
    print("Could not find any barcode.")
```
