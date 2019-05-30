# Python bindings for zxing-cpp (experimental)

## Installation

```bash
python setup.py install
```

## Usage

```python
import cv2
import zxing

img = cv2.imread('myimage.png')
result = zxing.decode(img)
if result.valid:
    print("Found barcode with value '{}' (format: {})".format(result.text, str(result.format))) 
else:
    print("could not read barcode")
```
