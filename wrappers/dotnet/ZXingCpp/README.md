# ZXingCpp

ZXingCpp is a high-performance .NET barcode reader and writer powered by [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp),
with a modern API. It supports all major linear and matrix barcode formats like QRCode, DataMatrix, PDF417, EAN/UPC, Code128,
etc. It is 5x-50x faster than ZXing.NET, supports more barcode formats (symbologies) and has a higher detection rate on difficult images.

## Install

```sh
dotnet add package ZXingCpp
dotnet add package SkiaSharp
```

## Read barcodes

SkiaSharp is used here because it works consistently across Windows, Linux, and macOS.

```cs
using SkiaSharp;
using ZXingCpp;

var img = SKBitmap.Decode(args[0]).Copy(SKColorType.Gray8);
var iv = new ImageView(img.GetPixels(), img.Info.Width, img.Info.Height, ImageFormat.Lum);

var readBarcodes = new BarcodeReader {
    Formats = args.Length > 1 ? BarcodeFormats.Parse(args[1]) : BarcodeFormat.All,
    TryInvert = false,
    // see the ReaderOptions for more available options
};

foreach (var b in readBarcodes.From(iv))
    Console.WriteLine($"{b.Format}: {b.Text}");
```

Executing this sample code from the command line would look like this:
```sh
dotnet run -- <image-file-name> [barcode-format-list]
```

## Write barcodes

```cs
using ZXingCpp;

var barcode = new Barcode(args[1], BarcodeFormat.Parse(args[0]));
File.WriteAllText(args[2], barcode.ToSVG());
```

Executing this sample code from the command line would look like this:
```sh
dotnet run -- <barcode-format> <text> <out-svg-file-name>
```

## Why ZXingCpp

- Fast native-core decoding and encoding (5x-50x faster than ZXing.Net).
- Strong real-world detection performance, especially for QRCode.
- Broad [format support](https://github.com/zxing-cpp/zxing-cpp/tree/master#supported-formats) (QRCode, DataMatrix, PDF417, EAN/UPC, Code128, and more).
- Standards-compliant binary and ECI handling (`bytesECI()`).
- Apache-2.0 licensed and free for commercial use ([Commercial Support](https://github.com/zxing-cpp/zxing-cpp/blob/master/Commercial%20Support.md) available).

For comparative results with other .NET barcode libraries, see [zxing-bench](https://github.com/axxel/zxing-bench/dotnet).

## Platform support

The NuGet package includes native libraries for x64 and arm64 on Windows, Linux, and macOS.
Version 0.5.3 includes [zxing-cpp-v3.1.1](https://github.com/zxing-cpp/zxing-cpp/releases#release-v3.1.1)

If you need to override native loading, ensure .NET can find `[lib]ZXing[.dll|.so|.dylib]`
(for example via `PATH` on Windows or `LD_LIBRARY_PATH` on Linux).

## More examples

- [ZXingCpp.DemoReader](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/dotnet/ZXingCpp.DemoReader/Program.cs)
- [ZXingCpp.DemoWriter](https://github.com/zxing-cpp/zxing-cpp/blob/master/wrappers/dotnet/ZXingCpp.DemoWriter/Program.cs)
