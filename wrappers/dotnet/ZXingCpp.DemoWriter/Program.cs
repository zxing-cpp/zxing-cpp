/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

using System.Collections.Generic;
using SkiaSharp;
using ZXingCpp;

public static class SkBitmapBarcodeWriter
{
	public static SKBitmap Write(Barcode barcode, WriterOptions? opts = null)
	{
		var img = barcode.ToImage(opts);
		var info = new SKImageInfo(img.Width, img.Height, SKColorType.Gray8);
		var res = new SKBitmap();
		res.InstallPixels(info, img.Data, img.Width, (IntPtr _, object _) => img.Dispose());
		return res;
	}

	public static SKBitmap ToSKBitmap(this Barcode barcode, WriterOptions? opts = null) => Write(barcode, opts);
}

public class Program
{
	public static void Main(string[] args)
	{
		var (format, text, fn) = (args[0], args[1], args[2]);

		var bc = new Barcode(text, Barcode.FormatFromString(format));

		var img = bc.ToImage();
		Console.WriteLine($"{img.Data}, {img.Width}, {img.Height}, {img.ToArray()}");

		if (fn.EndsWith(".svg"))
			File.WriteAllText(fn, bc.ToSVG());
		else
			using (SKBitmap skb = bc.ToSKBitmap(new WriterOptions(){Scale = 5})) {
				skb.Encode(new SKFileWStream(args[2]), SKEncodedImageFormat.Png, 100);
			}
	}

}
