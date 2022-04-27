# Python bindings for zxing-cpp

[![Build + Deploy](https://github.com/nu-book/zxing-cpp/actions/workflows/python-build.yml/badge.svg)](https://github.com/nu-book/zxing-cpp/actions/workflows/python-build.yml)
[![PyPI](https://img.shields.io/pypi/v/zxing-cpp.svg)](https://pypi.org/project/zxing-cpp/)

## Installation

```bash
pip install zxing-cpp
```
or

```bash
python setup.py install
```

## Usage

```python
import cv2
import zxingcpp

img = cv2.imread('myimage.png')
results = zxingcpp.read_barcodes(img)
for result in results:
	print("Found barcode:\n Text:    '{}'\n Format:   {}\n Position: {}"
		.format(result.text, result.format, result.position))
if len(results) == 0:
	print("Could not find any barcode.")
```
