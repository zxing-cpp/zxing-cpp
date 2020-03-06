# Python bindings for zxing-cpp (experimental)

## Installation

```bash
python setup.py install
```

## Usage

```python
import cv2
import zxing

image = zxing.encode('Test', width=100, height=100, format=zxing.BarcodeFormat.PDF_417)
cv2.imwrite('barcode.png', image)

img = cv2.imread('barcode.png')
result = zxing.decode(img)
if result.valid:
    print("Found barcode with value '{}' (format: {})".format(result.text, str(result.format))) 
else:
    print("could not read barcode")
```
