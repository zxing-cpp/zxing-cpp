[![Build Status](https://github.com/zxing-cpp/zxing-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/zxing-cpp/zxing-cpp/actions?query=workflow%3ACI)

# ZXing-C++

ZXing-C++ ("zebra crossing") is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.

It was originally ported from the Java [ZXing library](https://github.com/zxing/zxing) but has been developed further and now includes many improvements in terms of runtime and detection performance. It can both read and write barcodes in a number of formats. Since version 3.0 the default writing backend is provided by the [zint library](https://sourceforge.net/projects/zint/).

## Sponsors

You can sponsor this library at [GitHub Sponsors](https://github.com/sponsors/axxel).

Named Sponsors:
* [KURZ Digital Solutions GmbH & Co. KG](https://github.com/kurzdigital)
* [Useful Sensors Inc](https://github.com/usefulsensors)
* [synedra](https://synedra.com/)

Thanks a lot for your contribution!

## Features

* Written in pure C++20 (public API is C++17 compatible), no third-party dependencies (for the library itself)
* Thread safe
* Wrappers/Bindings for:
  * [Android](wrappers/android/README.md)
  * [C](wrappers/c/README.md)
  * [iOS](wrappers/ios/README.md)
  * [Kotlin/Native](wrappers/kn/README.md)
  * [.NET](wrappers/dotnet/README.md)
  * [Python](wrappers/python/README.md)
  * [Rust](wrappers/rust/README.md)
  * [WebAssembly](wrappers/wasm/README.md)
  * [WinRT](wrappers/winrt/README.md)
  * [Flutter](https://pub.dev/packages/flutter_zxing) (external project)

## Supported Formats

| Linear product  | Linear industrial | Matrix             |
|-----------------|-------------------|--------------------|
| UPC-A           | Code 39           | QR Code            |
| UPC-E           | Code 93           | Micro QR Code      |
| EAN-8           | Code 128          | rMQR Code          |
| EAN-13          | Codabar           | Aztec              |
| DataBar         | DataBar Expanded  | DataMatrix         |
| DataBar Limited | DX Film Edge      | PDF417             |
|                 | ITF               | MaxiCode (partial) |

[Note:]
 * DataBar used to be called RSS.
 * DataBar, DX Film Edge, MaxiCode, Micro QR Code and rMQR Code are not supported for writing if the library is configured with `ZXING_WRITERS=OLD`.

## Getting Started

### To read barcodes:
1. Load your image into memory (3rd-party library required).
2. Call `ReadBarcodes()` from [`ReadBarcode.h`](core/src/ReadBarcode.h), the simplest API to get a list of `Barcode` objects.

A very simple example looks like this:
```c++
#include "ZXing/ZXingCpp.h"
#include <iostream>

int main(int argc, char** argv)
{
    int width, height;
    unsigned char* data;
    // load your image data from somewhere. ImageFormat::Lum assumes grey scale image data.

    auto image = ZXing::ImageView(data, width, height, ZXing::ImageFormat::Lum);
    auto options = ZXing::ReaderOptions().formats(ZXing::BarcodeFormat::Any);
    auto barcodes = ZXing::ReadBarcodes(image, options);

    for (const auto& b : barcodes)
        std::cout << ZXing::ToString(b.format()) << ": " << b.text() << "\n";

    return 0;
}
```
To see the full capability of the API, have a look at [`ZXingReader.cpp`](example/ZXingReader.cpp).

### To write barcodes:
1. Create a `Barcode` object with `CreateBarcodeFrom...()` from [`CreateBarcode.h`](core/src/CreateBarcode.h).
2. The `Barcode::symbol()` can be used to get access to the bit matrix (1 module == 1 pixel, no quiet zone)
3. Alternatively the 3 `WriteBarcodeTo...()` functions from [`WriteBarcode.h`](core/src/WriteBarcode.h) can be used to create an `Image`, a SVG string or a UTF-8 string representation.

A very simple example looks like this:
```c++
#include "ZXing/ZXingCpp.h"
#include <iostream>

int main(int argc, char** argv)
{
    auto barcode = ZXing::CreateBarcodeFromText("some text", ZXing::BarcodeFormat::QRCode);
    auto svg = ZXing::WriteBarcodeToSVG(barcode);

    // see also ZXing::WriteBarcodeToImage()

    std::cout << svg << "\n";

    return 0;
}
```

As an example for how to parameterize the process with `CreatorOptions` and `WriterOptions`, have a look at [`ZXingWriter.cpp`](example/ZXingWriter.cpp).

## Web Demos
- [Read barcodes](https://zxing-cpp.github.io/zxing-cpp/demo_reader.html)
- [Write barcodes](https://zxing-cpp.github.io/zxing-cpp/demo_writer.html)
- [Read barcodes from camera](https://zxing-cpp.github.io/zxing-cpp/demo_cam_reader.html)

[Note: those live demos are not necessarily fully up-to-date at all times.]

## Build Instructions
These are the generic instructions to build the library on Windows/macOS/Linux. For details on how to build the individual wrappers, follow the links above.

1. Make sure [CMake](https://cmake.org) version 3.16 or newer is installed. The python module requires 3.18 or higher.
2. Make sure a sufficiently C++20 compliant compiler is installed (minimum VS 2019 16.10? / gcc 11 / clang 12?).
3. See the cmake `ZXING_...` options to enable the testing code, python wrapper, etc.

```
git clone https://github.com/zxing-cpp/zxing-cpp.git --recursive --single-branch --depth 1
cmake -S zxing-cpp -B zxing-cpp.release -DCMAKE_BUILD_TYPE=Release
cmake --build zxing-cpp.release -j8 --config Release
```

[Note: binary packages are available for/as
[vcpkg](https://github.com/Microsoft/vcpkg/tree/master/ports/nu-book-zxing-cpp),
[conan](https://github.com/conan-io/conan-center-index/tree/master/recipes/zxing-cpp),
[mingw](https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-zxing-cpp) and a bunch of
[linux distributions](https://repology.org/project/zxing-cpp/versions).]
