# ZXing-C++

This project is a C++ port of ZXing Library (https://github.com/zxing/zxing).

## Features

* In pure C++11, no third-party dependencies
* Stateless, thread-safe readers
* Wrapper to create WinRT component
* Wrapper for Android NDK

### Work in progress

Currently all readers are working with all original blackbox tests (from ZXing project) passed.
Encoders are comming soon.

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
