# ZXingCpp

ZXingCpp is a .NET wrapper for the C++ library [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

It is an open-source, multi-format linear/matrix barcode image processing library implemented in C++.
It was originally ported from the Java ZXing Library but has been developed further and now includes
many improvements in terms of runtime and detection performance.

## Usage

```cs
using System.Collections.Generic;
using ImageMagick;
using ZXingCpp;

// BarcodeReader extension class to support direct reading from MagickImage
public static class MagickImageBarcodeReader
{
    public static List<Barcode> Read(MagickImage img, ReaderOptions? opts = null)
    {
        if (img.DetermineBitDepth() < 8)
            img.SetBitDepth(8);
        var bytes = img.ToByteArray(MagickFormat.Gray);
        var iv = new ImageView(bytes, img.Width, img.Height, ImageFormat.Lum);
        return BarcodeReader.Read(iv, opts);
    }

    public static List<Barcode> Read(this BarcodeReader reader, MagickImage img)
        => Read(img, reader);
}

public class Program
{
    public static void Main(string[] args)
    {
        var img = new MagickImage(args[0]);

        var reader = new BarcodeReader() {
            Formats = BarcodeReader.FormatsFromString(args[1]),
            TryInvert = false,
            // see the ReaderOptions implementation for more available options
        };

        foreach (var b in reader.Read(img))
            Console.WriteLine($"{b.Format} : {b.Text}");
    }
}
```

To run the code above, it is important that the dotnet runtime finds the native
`ZXing[.dll|.so|.dylib]` in your path. E.g. on Linux a complete command line would look like this

```sh
LD_LIBRARY_PATH=<ZXing.so-path> dotnet run -- <image-file-name>
```

Note: This is an alpha release, meaning the API may still change slightly to feel even more
"managed" depending on community feedback.

## Benchmarking

To compare the performance of this .NET wrapper project with other available barcode scanner .NET libraries,
I started the project [zxing-bench](https://github.com/axxel/zxing-bench).
The [README](https://github.com/axxel/zxing-bench/blob/main/dotnet/README.md) contains a few results to get an idea.
