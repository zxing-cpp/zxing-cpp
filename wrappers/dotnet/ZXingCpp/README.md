# ZXingCpp

ZXingCpp is a .NET wrapper for the C++ library [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

It is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.
It was originally ported from the Java ZXing library but has been developed further and now includes
many improvements in terms of runtime and detection performance.


## Usage Reading

```cs
using SkiaSharp;
using ZXingCpp;

public class Program
{
    public static void Main(string[] args)
    {
        var img = SKBitmap.Decode(args[0]).Copy(SKColorType.Gray8);
        var iv = new ImageView(img.GetPixels(), img.Info.Width, img.Info.Height, ImageFormat.Lum);

        var readBarcodes = new BarcodeReader() {
            Formats = args.Length > 1 ? Barcode.FormatsFromString(args[1]) : BarcodeFormats.Any,
            TryInvert = false,
            // see the ReaderOptions implementation for more available options
        };

        foreach (var b in readBarcodes.From(iv))
            Console.WriteLine($"{b.Format} : {b.Text}");
    }
}
```

Executing this sample code from the command line would look like this:
```sh
dotnet run -- <image-file-name> [barcode-format-list]
```

See also the [ZXingCpp.DemoReader](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/dotnet/ZXingCpp.DemoReader/Program.cs)
which shows the use of extension classes to support SkiaSharp and ImageMagick based input.

The NuGet package includes the runtime/native c++ libraries for the x64 architecture on
Windows, Linux and macOS. If something is not working out of the box or you need arm64 support
then you need to build the `[lib]ZXing[.dll|.so|.dylib]` file yourself and make sure the .NET
runtime find it (see e.g. the environment variables `LD_LIBRARY_PATH` on Linux or `PATH` on
Windows).

Note: This is an alpha release, meaning the API may still change slightly to potentially feel even
more like a native C# library depending on community feedback.

## Usage Writing

```cs
using ZXingCpp;

public class Program
{
    public static void Main(string[] args)
    {
        var barcode = new Barcode(args[1], Barcode.FormatFromString(args[0]));
        File.WriteAllText(args[2], barcode.ToSVG());
    }
}
```

Executing this sample code from the command line would look like this:
```sh
dotnet run -- <barcode-format> <text> <out-svg-file-name>
```

For an example how to write a PNG file instead of a SVG file, have a look at the
[ZXingCpp.DemoWriter](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/dotnet/ZXingCpp.DemoWriter/Program.cs).

## Why ZXingCpp?

There are a number of areas where ZXingCpp shines compared to other popular .NET barcode scanner libraries.
The following comparison is with respect to the open source [ZXing.Net](https://www.nuget.org/packages/ZXing.Net)
and the commercial [Dynamsoft](https://www.nuget.org/packages/Dynamsoft.DotNet.Barcode) projects.

### Performance

To compare the performance of ZXingCpp with the other two libraries, I started the project
[zxing-bench](https://github.com/axxel/zxing-bench).
The [README](https://github.com/axxel/zxing-bench/blob/main/dotnet/README.md) contains a few details but to get
an idea: ZXingCpp is on average 2x-10x faster than Dynamsoft and 10x-50x faster than ZXing.Net.

### Detection rate

The benchmarking tool also showed that ZXingCpp has a superior detection rate compared to ZXing.Net while it is
sometimes better sometimes worse than the commercial Dynamsoft package, depending on the sample type and the
library configuration. The latter definitively supports more barcode formats compared to the two ZXing decendents.

### Ease of use

The sample program above shows the simplicitly of the API. The others are similar but seem a bit more
complicated with regards to setting parameters.

### Standards support

ZXingCpp has full support for binary data and ECI handling and provides a standards conforming `bytesECI()`
data that can be used to simulate a hardware/handheld barcode scanner. This seems not the case for ZXing.Net
and is unclear for Dynamsoft.

### License / costs

ZXingCpp has the liberal Apache-2.0 license and is free to use in commercial applications. That said,
I accept [donations](https://github.com/sponsors/axxel) and might be available for commercial consulting ;).
