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
result = zxingcpp.read_barcode(img)
if result.valid:
    print("Found barcode with value '{}' (format: {})".format(result.text, str(result.format))) 
else:
    print("could not read barcode")
```
