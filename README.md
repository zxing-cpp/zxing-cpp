[![Build Status](https://travis-ci.org/nu-book/zxing-cpp.svg?branch=master)](https://travis-ci.org/nu-book/zxing-cpp)

# ZXing-C++

This project is a C++ port of [ZXing Library](https://github.com/zxing/zxing).

## Features

* In pure C++14, no third-party dependencies
* Stateless, thread-safe readers/generators
* Wrapper to create WinRT component
* Wrapper for Android
* Wrapper for WebAssembly
* Python binding

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

## Getting Started

### To read barcodes:
As an example, have a look at [`ZXingReader.cpp`](example/ZXingReader.cpp).
1. Load your image into memory (3rd-party library required).
2. Call `ReadBarcode()` from [`ReadBarcode.h`](core/src/ReadBarcode.h), the simplest API to get a `Result`.

### To write barcodes:
As an example, have a look at [`ZXingWriter.cpp`](example/ZXingWriter.cpp).
1. Create a [`MultiFormatWriter`](core/src/MultiFormatWriter.h) instance with the format you want to generate. Set encoding and margins if needed.
2. Call `encode()` with text content and the image size. This returns a [`BitMatrix`](core/src/BitMatrix.h) which is a binary image of the barcode where `true` == visual black and `false` == visual white.
3. Convert the bit matrix to your native image format. See also the `ToMatrix<T>(BitMatrix&)` helper function.

## Web Demos
- [Read barcodes](https://nu-book.github.io/zxing-cpp/demo_reader.html)
- [Write barcodes](https://nu-book.github.io/zxing-cpp/demo_writer.html)
- [Scan with camera](https://nu-book.github.io/zxing-cpp/zxing_viddemo.html)

## WinRT Package
A nuget package is available for WinRT: [huycn.zxingcpp.winrt](https://www.nuget.org/packages/huycn.zxingcpp.winrt). 
To install it, run the following command in the Package Manager Console
```sh
PM> Install-Package huycn.zxingcpp.winrt
```

## Build Instructions

### Standard setup on Windows/macOS/Linux
1. Make sure [CMake](https://cmake.org) version 3.10 or newer is installed.
2. Make sure a C++14 compliant compiler is installed (minimum VS 2017 15.0 / gcc 5 / clang 3.4)
3. See the cmake `BUILD_...` options to enable the testing code, python wrapper, etc.

### Windows GDIPlus wrapper
1. Open CMake GUI, specify [`wrappers/gdiplus`](wrappers/gdiplus) as source folder in the first input, specify the build output in the second input, and click on Generate.
2. At prompt, select "Visual Studio 15 2017" (or "Visual Studio 15 2017 Win64" if you want to build for x64 platform); leave the second input (Optional toolset...) empty; leave "Use default native compilers" checked; and click on Finish to generate the VS project. At the end, you will get a solution (.sln) in your binary output directory that you can open in VS. The project ZXingGdiPlus in the solution will generate a static library.

### Windows Universal Platform
1. Download and install [CMake](https://cmake.org) 3.4 or more recent if it's not already installed.
2. Edit the file [`wrappers/winrt/BuildWinCom.bat`](wrappers/winrt/BuildWinCom.bat) to adjust the path to your CMake installation.
3. Double-click on the batch script to run it.
4. If the build succeeds, it will put the results in the folder UAP which is ready-to-use SDK extension.

### Android NDK
Note: The original Java-only [ZXing](https://github.com/zxing/zxing) project has a very good support for Android, whether you want to use it as external app via Intent or directly integrated into your app. You should consider using it first before trying this library since involving with native code is always more complex than Java-only code. Performance-wise, except for specific usecases, you won't notice the difference!

1. Edit [`wrappers/android/jni/Application.mk`](wrappers/android/jni/Application.mk) and adjust for your project.
2. On command line, `cd` into [`wrappers/android/jni`](wrappers/android/jni), type `ndk-build` (or `ndk-build -j <number of your CPU cores>`)
3. Copy files in `libs` and `java` into corresponding folders of your Android project.

### WebAssembly
1. [Install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/) if not done already.
2. In an empty build folder, invoke `emcmake cmake <path to zxing-cpp.git/wrappers/wasm>`.
3. Invoke `cmake --bulid .` to create `zxing.js` and `zxing.wasm` (and `_reader`/`_writer` versions).
4. Copy these two files to your web folder and create an HTML page that includes `zxing.js`.

You can also download the latest build output from the continuous integration system from the [Actions](https://github.com/nu-book/zxing-cpp/actions) tab. Look for 'wasm-artifacts'.

For usage examples see [reader](wrappers/wasm/demo_reader.html) and [writer](wrappers/wasm/demo_writer.html) demos or the [live demos](https://nu-book.github.io/zxing-cpp/).
