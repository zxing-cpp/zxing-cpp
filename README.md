[![Build Status](https://github.com/nu-book/zxing-cpp/workflows/CI/badge.svg?branch=master)](https://github.com/nu-book/zxing-cpp/actions?query=workflow%3ACI)

# ZXing-C++

ZXing-C++ ("zebra crossing") is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.

It was originally ported from the Java [ZXing Library](https://github.com/zxing/zxing) but has been developed further and now includes many improvements in terms of quality and performance. It can both read and write barcodes in a number of formats.

## Features

* In pure C++17, no third-party dependencies (for the library)
* Stateless, thread-safe readers/scanners and writers/generators
* Wrapper/Bindings for:
  * [Android](wrappers/android/README.md)
  * [iOS](wrappers/ios/README.md)
  * [Python](wrappers/python/README.md)
  * [WebAssembly](wrappers/wasm/README.md)
  * [WinRT](wrappers/winrt/README.md)
  * [Flutter](https://pub.dev/packages/flutter_zxing) (external project)

## Supported Formats

| Linear product | Linear industrial | Matrix             |
|----------------|-------------------|--------------------|
| UPC-A          | Code 39           | QR Code            |
| UPC-E          | Code 93           | Micro QR Code      |
| EAN-8          | Code 128          | Aztec              |
| EAN-13         | Codabar           | DataMatrix         |
| DataBar        | DataBar Exanded   | PDF417             |
|                | ITF               | MaxiCode (partial) |

Note: DataBar used to be called RSS. DataBar is not supported for writing.

## Getting Started

### To read barcodes:
As an example, have a look at [`ZXingReader.cpp`](example/ZXingReader.cpp).
1. Load your image into memory (3rd-party library required).
2. Call `ReadBarcodes()` from [`ReadBarcode.h`](core/src/ReadBarcode.h), the simplest API to get a list of `Result` objects.

### To write barcodes:
As an example, have a look at [`ZXingWriter.cpp`](example/ZXingWriter.cpp).
1. Create a [`MultiFormatWriter`](core/src/MultiFormatWriter.h) instance with the format you want to generate. Set encoding and margins if needed.
2. Call `encode()` with text content and the image size. This returns a [`BitMatrix`](core/src/BitMatrix.h) which is a binary image of the barcode where `true` == visual black and `false` == visual white.
3. Convert the bit matrix to your native image format. See also the `ToMatrix<T>(BitMatrix&)` helper function.

## Web Demos
- [Read barcodes](https://nu-book.github.io/zxing-cpp/demo_reader.html)
- [Write barcodes](https://nu-book.github.io/zxing-cpp/demo_writer.html)
- [Scan with camera](https://nu-book.github.io/zxing-cpp/zxing_viddemo.html)

## Build Instructions (for Windows/macOS/Linux)
1. Make sure [CMake](https://cmake.org) version 3.14 or newer is installed.
2. Make sure a C++17 compliant compiler is installed (minimum VS 2019 16.8 / gcc 7 / clang 5).
3. See the cmake `BUILD_...` options to enable the testing code, python wrapper, etc.

