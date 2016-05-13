# About

This project is a C++ port of ZXing Library (https://github.com/zxing/zxing).

## Features

* In pure C++11, no third-party dependencies
* Stateless, thread-safe readers
* Built as UWP WinRT component

### Work in progress

Currently all readers are working with all original blackbox tests (from ZXing project) passed.
Encoders are comming soon.
More wrappers comming soon.

## Supported Formats

Same as ZXing, following barcode are supported:

| 1D product | 1D industrial | 2D
| ---------- | ------------- | --------------
| UPC-A      | Code 39       | QR Code
| UPC-E      | Code 93       | Data Matrix
| EAN-8      | Code 128      | Aztec (beta)
| EAN-13     | Codabar       | PDF 417 (beta)
|            | ITF           |
|            | RSS-14        |
|            | RSS-Expanded  |
