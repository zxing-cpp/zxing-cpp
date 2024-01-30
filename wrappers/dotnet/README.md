# ZXingCpp

ZXingCpp is a .NET wrapper for the C++ library [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

It is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.
It was originally ported from the Java ZXing Library but has been developed further and now includes
many improvements in terms of runtime and detection performance.

## Usage

There is a NuGet package available: https://www.nuget.org/packages/ZXingCpp. It does currently not yet
contain the native binary dll file. That needs to be copied/build separately at the moment.

Simple example usage:

```cs
using System.Collections.Generic;
using ImageMagick;
using ZXingCpp;

public static class ImageMagickBarcodeReader
{
    public static List<Barcode> Read(MagickImage img, ReaderOptions? opts = null)
    {
        var bytes = img.ToByteArray(MagickFormat.Gray);
        var iv = new ImageView(bytes, img.Width, img.Height, ImageFormat.Lum, 0, 0);
        return BarcodeReader.Read(iv, opts);
    }

    public static List<Barcode> Read(this BarcodeReader reader, MagickImage img) => Read(img, reader);
}

public class Program
{
    public static void Main(string[] args)
    {
        var img = new MagickImage(args[0]);

        var reader = new BarcodeReader() {
            Formats = BarcodeReader.FormatsFromString(args[1]),
            TryInvert = false,
        };

        foreach (var b in reader.Read(img))
            Console.WriteLine($"{b.Format} : {b.Text}");
    }
}
```

To run the `ZXingCppDemo` sample program, it is important that the dotnet runtime finds the native
`ZXing[.dll|.so|.dylib]` in your path. E.g. on Linux a complete command line would look like this

```sh
LD_LIBRARY_PATH=<path-to-your-ZXing.so> dotnet run --project ZXingCppDemo -- ../../test/samples/multi-1/1.png
```

Note: This should currently be considered a pre-release. The API may change slightly to be even more
"managed" depending on community feedback.

## Benchmarking

To compare the performance of this .NET wrapper project with other available barcode scanner .NET libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench). The README contains a few
results to get an idea.
