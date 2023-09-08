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

[Note: To install via `setup.py` (or via `pip install` in case there is no pre-build wheel available for your platfor or python version), you need a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a c++ compiler.]

## Usage

```python
import cv2, zxingcpp

img = cv2.imread('test.png')
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

To get a full list of available parameters for `read_barcodes` and `write_barcode` as well as the properties of the result objects, have a look at the `PYBIND11_MODULE` definition in [this c++ source file](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/python/zxing.cpp).
