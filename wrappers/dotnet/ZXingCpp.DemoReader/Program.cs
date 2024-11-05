/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

using System.Collections.Generic;
using ImageMagick;
using SkiaSharp;
using ZXingCpp;

public static class MagickImageBarcodeReader
{
	public static Barcode[] Read(MagickImage img, ReaderOptions? opts = null)
	{
		if (img.DetermineBitDepth() < 8)
			img.SetBitDepth(8);
		var bytes = img.ToByteArray(MagickFormat.Gray);
		var iv = new ImageView(bytes, img.Width, img.Height, ImageFormat.Lum);
		return BarcodeReader.Read(iv, opts);
	}

	public static Barcode[] From(this BarcodeReader reader, MagickImage img) => Read(img, reader);
}

public static class SkBitmapBarcodeReader
{
	public static Barcode[] Read(SKBitmap img, ReaderOptions? opts = null)
	{
		var format = img.Info.ColorType switch
		{
			SKColorType.Gray8 => ImageFormat.Lum,
			SKColorType.Rgba8888 => ImageFormat.RGBA,
			SKColorType.Bgra8888 => ImageFormat.BGRA,
			_ => ImageFormat.None,
		};
		if (format == ImageFormat.None)
		{
			if (!img.CanCopyTo(SKColorType.Gray8))
				throw new Exception("Incompatible SKColorType");
			img = img.Copy(SKColorType.Gray8);
			format = ImageFormat.Lum;
		}
		var iv = new ImageView(img.GetPixels(), img.Info.Width, img.Info.Height, format);
		return BarcodeReader.Read(iv, opts);
	}

	public static Barcode[] From(this BarcodeReader reader, SKBitmap img) => Read(img, reader);
}

public class Program
{
	public static void Main(string[] args)
	{
#if false
		var img = new MagickImage(args[0]);
#else
		var img = SKBitmap.Decode(args[0]);
#endif
		Console.WriteLine(img);

		var readBarcodes = new BarcodeReader() {
			TryInvert = false,
			ReturnErrors = true,
		};

		if (args.Length >= 2)
			readBarcodes.Formats = Barcode.FormatsFromString(args[1]);

		foreach (var b in readBarcodes.From(img))
			Console.WriteLine($"{b.Format} ({b.ContentType}): {b.Text} / [{string.Join(", ", b.Bytes)}] {b.ErrorMsg}");
	}
}
