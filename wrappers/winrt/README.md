# WinRT Package/Wrapper

## Install

A nuget package is available for WinRT: [huycn.zxingcpp.winrt](https://www.nuget.org/packages/huycn.zxingcpp.winrt). 
To install it, run the following command in the Package Manager Console
```sh
PM> Install-Package huycn.zxingcpp.winrt
```

## Build

1. Make sure [CMake](https://cmake.org) version 3.4 or newer is installed.
2. Make sure a C++17 compliant compiler is installed (minimum VS 2019 16.8).
3. Edit the file [`BuildWinCom.bat`](BuildWinCom.bat) to adjust the path to your CMake installation.
4. Double-click on the batch script to run it.
5. If the build succeeds, it will put the results in the folder UAP which is ready-to-use SDK extension.

