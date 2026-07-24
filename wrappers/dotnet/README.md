# ZXingCpp

ZXingCpp is a .NET wrapper for [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

There is a NuGet package available: https://www.nuget.org/packages/ZXingCpp.

## Usage

See either the [ZXingCpp/README.md](ZXingCpp/README.md) or the [ZXingCpp.DemoReader](ZXingCpp.DemoReader) or [ZXingCpp.DemoWriter](ZXingCpp.DemoWriter) project.

To run the `ZXingCpp.DemoReader` sample program, it is important that the dotnet runtime finds the native
`ZXing[.dll|.so|.dylib]` in your path. E.g. on Linux a complete command line to execute the demo from a
local clone of the repository would look like this:

```sh
LD_LIBRARY_PATH=<ZXing.so-path> dotnet run --project ZXingCpp.DemoReader -- ../../test/samples/multi-1/1.png
```

The NuGet package comes with native libraries for x64 and arm64 for Windows, Linux and macOS.

## Benchmarking

To compare the performance of this .NET wrapper project with other available barcode scanner .NET libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench). The README contains a few
results to get an idea.
