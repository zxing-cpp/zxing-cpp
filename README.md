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

## Web Demos
- [Scan barcodes](https://nu-book.github.io/zxing-cpp/demo_reader.html)
- [Generate barcodes](https://nu-book.github.io/zxing-cpp/demo_writer.html)
- [Scan with camera](https://nu-book.github.io/zxing-cpp/zxing_viddemo.html)


## WinRT Package
A nuget package is available for WinRT: [huycn.zxingcpp.winrt](https://www.nuget.org/packages/huycn.zxingcpp.winrt). 
To install it, run the following command in the Package Manager Console
```sh
PM> Install-Package huycn.zxingcpp.winrt
```

## Getting Started
The wrappers export very simple API to use, check `BarcodeReader` and `BarcodeGenerator` in each wrapper.
For more fine-grain control in scanning process, check [`MultiFormatReader`](core/src/MultiFormatReader.h) class. For more customization when generating particular barcode format, you need to instantiate appropriate writer, see [`MultiFormatWriter`](core/src/MultiFormatWriter.h) for more details.


## Build Instructions
### For Windows Desktop with VS
1. Download and install [CMake](https://cmake.org) if it's not already installed.
2. Open CMake GUI, specify [`wrappers/gdiplus`](wrappers/gdiplus) as source folder in the first input, specify the build output in the second input, and click on Generate.
3. At prompt, select "Visual Studio 14 2015" (or "Visual Studio 14 2015 Win64" if you want to build for x64 platform); leave the second input (Optional toolset...) empty; leave "Use default native compilers" checked; and click on Finish to generate the VS project. At the end, you will get a solution (.sln) in your binary output directory that you can open in VS. The project ZXingGdiPlus in the solution will generate a static library.

### For Windows Universal Platform
1. Download and install [CMake](https://cmake.org) 3.4 or more recent if it's not already installed.
2. Edit the file [`wrappers/winrt/BuildWinCom.bat`](wrappers/winrt/BuildWinCom.bat) to adjust the path to your CMake installation.
3. Double-click on the batch script to run it.
4. If the build succeeds, it will put the results in the folder UAP which is ready-to-use SDK extension.

### For Android NDK
Note: The original Java-only [ZXing](https://github.com/zxing/zxing) project has a very good support for Android, whether you want to use it
as external app via Intent or directly integrated into your app. You should consider using it first before
trying this library since involving with native code is always more complex than Java-only code. Performance-wise, 
except for specific usecases, you won't notice the difference!

1. Edit [`wrappers/android/jni/Application.mk`](wrappers/android/jni/Application.mk) and adjust for your project.
2. On command line, `cd` into [`wrappers/android/jni`](wrappers/android/jni), type `ndk-build` (or `ndk-build -j <number of your CPU cores>`)
3. Copy files in `libs` and `java` into corresponding folders of your Android project.

### For web browser (WebAssembly)
1. [Install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/) if not done already.
2. In a empty build folder, invoke `cmake` from `emconfigure` create Makefile, using [`wrappers/wasm/Toolchain-Emscripten.cmake`](wrappers/wasm/Toolchain-Emscripten.cmake) as toolchain file. For example:
```
EMSCRIPTEN_PATH=<path to your Emscripten installation, e.g. ~/emsdk/emscripten/tag-1.38.21>
SOURCE_BASEDIR=<zxing-cpp-dir/wrappers/wasm>
emconfigure cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$SOURCE_BASEDIR/Toolchain-Emscripten.cmake" -DEMSCRIPTEN_ROOT_PATH="$EMSCRIPTEN_PATH" "$SOURCE_BASEDIR"
```
3. Invoke emmake to create `zxing.js` and `zxing.wasm`
```
emmake make
```
4. Copy these two files to your web folder and create HTML page that includes `zxing.js`.

See usage example of exported functions from [demos](https://nu-book.github.io/zxing-cpp/).

By default, both encoder and decoder are included. If you don't plan to use either of them, you can disable it to reduce generated code size. To do so, in the line `emconfigure cmake ...` above, pass `-DENABLE_ENCODERS=0` to disable encoders or `-DENABLE_DECODERS=0` to disable decoders.

### For other platforms
Wrappers are provided as convenient way to work with native image format. You still can use the library without a wrapper.

##### To read barcodes:
As an example, have a look at [`scan_image.cpp`](example/scan_image.cpp).
1. Create a [`LuminanceSource`](core/src/LuminanceSource.h) instance. This interface abstracts an image source. You will need a third-party library to read your images. If you already have an image uncompressed in memory and you know its layout, you can easily go with [`GenericLuminanceSource`](core/src/GenericLuminanceSource.h). Otherwise, you will need to come up with your own implementation of the interface.
2. Use the `LuminanceSource` instance above to create an instance of [`BinaryBitmap`](core/src/BinaryBitmap.h). You have choices between [`HybridBinarizer`](core/src/HybridBinarizer.h) or [`GlobalHistogramBinarizer`](core/src/GlobalHistogramBinarizer.h). See class document in header files for more details on theses choices.
3. Create an instance of [`MultiFormatReader`](core/src/MultiFormatReader.h) with appropriate hints. Pay attention to `possibleFormats()`, `tryHarder()` and `tryRotate()`. These parameters will affect accuracy as well as reader's speed.
4. Call `MultiFormatReader::read()` with the `BinaryBitmap` created above to read your barcodes.

##### To write barcodes:
As an example, have a look at [`generate_image.cpp`](example/generate_image.cpp).
1. Create a [`MultiFormatWriter`](core/src/MultiFormatWriter.h) instance with the format you want to generate. Set encoding and margins if needed.
2. Call `encode()` with text content and the image size. This returns a [`BitMatrix`](core/src/BitMatrix.h) which is a binary image of the barcode where `true` == visual black and `false` == visual white.
3. Convert the bit matrix to your native image format. See also the `toByteMatrix` helper method.
