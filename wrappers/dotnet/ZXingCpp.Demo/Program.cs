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
	public static List<Barcode> Read(MagickImage img, ReaderOptions? opts = null)
	{
		if (img.DetermineBitDepth() < 8)
			img.SetBitDepth(8);
		var bytes = img.ToByteArray(MagickFormat.Gray);
		var iv = new ImageView(bytes, img.Width, img.Height, ImageFormat.Lum);
		return BarcodeReader.Read(iv, opts);
	}

	public static List<Barcode> Read(this BarcodeReader reader, MagickImage img) => Read(img, reader);
}

public static class SkBitmapBarcodeReader
{
	public static List<Barcode> Read(SKBitmap img, ReaderOptions? opts = null)
	{
		var format = img.Info.ColorType switch
		{
			SKColorType.Gray8 => ImageFormat.Lum,
			SKColorType.Rgba8888 => ImageFormat.RGBX,
			SKColorType.Bgra8888 => ImageFormat.BGRX,
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

	public static List<Barcode> Read(this BarcodeReader reader, SKBitmap img) => Read(img, reader);
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

		var reader = new BarcodeReader() {
			TryInvert = false,
		};

		if (args.Length >= 2)
			reader.Formats = BarcodeReader.FormatsFromString(args[1]);
	
		foreach (var b in reader.Read(img))
			Console.WriteLine($"{b.Format} ({b.ContentType}): {b.Text} / [{string.Join(", ", b.Bytes)}]");
	}
}
