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

**Note**: To enable position independent and multi-symbol DataMatrix detection, the library needs to be compiled with a c++20 compiler. Unfortunately some build environments (currently the 32-bit builds for Linux) used by `cibuildwheel` to generate the binary wheels that are published on [pypi.org](https://pypi.org/project/zxing-cpp/) don't include a c++20 compiler. Best chance to enable proper DataMatrix support in that case is by installing from source:

```bash
pip install zxing-cpp --no-binary zxing-cpp
```

In that case or if there is no pre-build wheel available for your platform or python version or if you use `setup.py` directly, a suitable [build environment](https://github.com/zxing-cpp/zxing-cpp#build-instructions) including a c++ compiler is required.


## Usage

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

To get a full list of available parameters for `read_barcodes` and `write_barcode` as well as the properties of the Barcode objects, have a look at the `PYBIND11_MODULE` definition in [this c++ source file](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/python/zxing.cpp).
