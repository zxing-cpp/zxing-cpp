# ZXingCpp

ZXingCpp is a .NET wrapper for [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

There is a NuGet package available: https://www.nuget.org/packages/ZXingCpp. It does currently not yet
contain the native binary dll file. That needs to be copied/build separately at the moment.

## Usage

See either the [ZXingCpp/README.md](ZXingCpp/README.md) or the [ZXingCpp.Demo](ZXingCpp.Demo) project.

To run the `ZXingCpp.Demo` sample program, it is important that the dotnet runtime finds the native
`ZXing[.dll|.so|.dylib]` in your path. E.g. on Linux a complete command line would look like this

```sh
LD_LIBRARY_PATH=<ZXing.so-path> dotnet run --project ZXingCpp.Demo -- ../../test/samples/multi-1/1.png
```

## Benchmarking

To compare the performance of this .NET wrapper project with other available barcode scanner .NET libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench). The README contains a few
results to get an idea.
