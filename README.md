# ZXing-C++

This project is a C++ port of ZXing Library (https://github.com/zxing/zxing).

## Features

* In pure C++11, no third-party dependencies
* Stateless, thread-safe readers/generators
* Wrapper to create WinRT component
* Wrapper for Android NDK

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

## Install
A nuget package is available for WinRT: https://www.nuget.org/packages/huycn.zxingcpp.winrt
To install it, run the following command in the Package Manager Console
```sh
PM> Install-Package huycn.zxingcpp.winrt
```

## Getting Started
The wrappers export very simple API to use, check BarcodeReader and BarcodeGenerator in each wrapper.
For more fine-grain control in scanning process, check MultiFormatReader class. For more customization when generating particular barecode format, you need to instantiate appropriate writer, see MultiFormatWriter for more details.


## Build instructions
### For Windows Desktop with VS
1. Download and install CMake (https://cmake.org) if it's not already installed.
2. Open CMake GUI, specify "<path_to_code_source>\wrappers\gdiplus" as source folder in the first input, specify the build output in the second input, and click on Generate.
3. At prompt, select "Visual Studio 14 2015" (or "Visual Studio 14 2015 Win64" if you want to build for x64 platform); leave the second input (Optional toolset...) empty; leave "Use default native compilers" checked; and click on Finish to generate the VS project. At the end, you will get a solution (.sln) in your binary output directory that you can open in VS. The project ZXingGdiPlus in the solution will generate a static library.

### For Windows Universal Platform
1. Download and install CMake (https://cmake.org) 3.4 or more recent if it's not already installed.
2. Edit the file wrappers\winrt\BuildWinCom.bat to adjust the path to your CMake installation.
3. Double-click on the batch script to run it.
4. If the build succeeds, it will put the results in the folder UAP which is ready-to-use SDK extension.

### For Android NDK
Note: The original Java-only ZXing project has a very good support for Android, whether you want to use it
as external app via Intent or directly integrated into your app. You should consider using it first before
trying this library since involving with native code is always more complex than Java-only code. Performance-wise, 
except for specific usecases, you won't notice the difference!

1. Edit wrappers/android/jni/Application.mk and adjust for your project.
2. On command line, being in wrappers/android, type `ndk-build` (or `ndk-build -j <number of your CPU cores>`)
3. Copy files in `libs` and `java` into corresponding folders of your Android project.

### For other platforms
Wrappers are provided as convenient way to work with native image format. You still can use the library without a wrapper.

##### To read barcodes:
1. Create a `LuminanceSource` instance. This interface abstracts image source. You will need a third-party library to read your images. If you already have the image uncompressed in memory and you know its layout, you can easily go with `GenericLuminanceSource`. Otherwise, you will need to come up with your own implementation of the interface.
2. Use the `LuminanceSource` instance above to create an instance of `BinaryBitmap`. You have choices between `HybridBinarizer` or `GlobalHistogramBinarizer`. See class document in header files for more details on theses choices.
3. Create an instance of `MultiFormatReader` with appropriate hints. Pay attention to `possibleFormats()`, `shouldTryHarder()`, `shouldTryRotate()`. These parameters will affect accuracy as well as reader's speed.
4. Call `MultiFormatReader::read()` with the `BinaryImage` created above to read your barcodes.

##### To write barcodes:
1. Create a `MultiFormatWriter` instance with the format you want to generate. Set encoding and margins if needed.
2. Call `encode()` with text content and the image size. This returns an `BitMatrix` which kind of binary image of the barcode where `true` == visual black and `false` == visual white.
3. Convert the bit matrix to your native image format.

