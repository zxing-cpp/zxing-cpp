using Xunit;
using System.Text;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using ZXingCpp;

namespace ZXingCpp.Tests;

public class UnitTest1
{
	[Fact]
	public void ValidBarcodeFormatParsing()
	{
		Assert.Equal(BarcodeFormat.QRCode, BarcodeFormat.Parse("qrcode"));
		Assert.Equal(BarcodeFormat.AllLinear, BarcodeFormat.Parse("AllLinear"));
	}

	[Fact]
	public void InvalidBarcodeFormatParsing()
	{
		Assert.Throws<Exception>(() => BarcodeFormat.Parse("nope"));
	}

	[Fact]
	public void InvalidImageBuffers()
	{
		Assert.Throws<Exception>(() => BarcodeReader.Read(Array.Empty<byte>(), 1, 1, ImageFormat.Lum));
		var error = Assert.Throws<Exception>(() => BarcodeReader.Read(new byte[1], 1, 1, ImageFormat.Lum, rowStride: 2));
		Assert.Contains("out of bounds", error.Message);
		Assert.Contains("NULL pointer", Assert.Throws<Exception>(() => BarcodeReader.Read(IntPtr.Zero, 1, 1, 1, ImageFormat.Lum)).Message);
	}

	[Fact]
	public void InvalidBarcodeCreationReportsNativeError()
	{
		using var options = new CreatorOptions(BarcodeFormat.None);
		var error = Assert.Throws<Exception>(() => new Barcode("hello", options));
		Assert.Contains("Unsupported barcode format", error.Message);
	}

	private static byte[] Ean8Pixels() => "0000101000101101011110111101011011101010100111011100101000100101110010100000"
		.Select(value => (byte)(value == '0' ? 255 : 0)).ToArray();

	[MethodImpl(MethodImplOptions.NoInlining)]
	private static (byte[] Buffer, WeakReference<byte[]> WeakBuffer) CreateManagedBuffer()
	{
		var pixels = Ean8Pixels();
		return (pixels, new WeakReference<byte[]>(pixels));
	}

	[Fact]
	public void ReadArrayAfterCollection()
	{
		var (pixels, weakBuffer) = CreateManagedBuffer();
		GC.Collect(2, GCCollectionMode.Forced, true, true);
		GC.WaitForPendingFinalizers();
		Assert.True(weakBuffer.TryGetTarget(out _));

		using var reader = new BarcodeReader { Binarizer = Binarizer.BoolCast };
		var barcodes = reader.Read(pixels, pixels.Length, 1, ImageFormat.Lum);
		using var barcode = Assert.Single(barcodes);
		Assert.Equal("96385074", barcode.Text);

		Array.Fill(pixels, (byte)255);
		Assert.Empty(reader.Read(pixels, pixels.Length, 1, ImageFormat.Lum));
	}

	[Fact]
	public void ReadDuringCollections()
	{
		var pixels = Ean8Pixels();
		using var cancellation = new CancellationTokenSource();
		using var started = new ManualResetEventSlim();
		var collector = Task.Run(() => {
			started.Set();
			while (!cancellation.IsCancellationRequested) {
				GC.Collect(2, GCCollectionMode.Forced, true, true);
				GC.WaitForPendingFinalizers();
			}
		});
		started.Wait();
		try {
			for (int index = 0; index < 100; ++index) {
				using var reader = new BarcodeReader { Binarizer = Binarizer.BoolCast };
				var results = reader.Read(pixels, pixels.Length, 1, ImageFormat.Lum);
				using var barcode = Assert.Single(results);
				Assert.Equal("96385074", barcode.Text);
			}
		} finally {
			cancellation.Cancel();
			collector.GetAwaiter().GetResult();
		}
	}

	[Fact]
	public void DeprecatedImageViewStillWorksWhenPinned()
	{
		var pixels = Ean8Pixels();
		var pin = GCHandle.Alloc(pixels, GCHandleType.Pinned);
		try {
#pragma warning disable CS0618
			var view = new ImageView(pixels.AsSpan(), pixels.Length, 1, ImageFormat.Lum);
			using var reader = new BarcodeReader { Binarizer = Binarizer.BoolCast };
			using var barcode = Assert.Single(reader.From(view));
			Assert.Equal("96385074", barcode.Text);
#pragma warning restore CS0618
		} finally {
			pin.Free();
		}
	}

	[Fact]
	public void ReadCallerOwnedPointer()
	{
		var pixels = Ean8Pixels();
		var pointer = Marshal.AllocHGlobal(pixels.Length);
		try {
			Marshal.Copy(pixels, 0, pointer, pixels.Length);
			using var reader = new BarcodeReader { Binarizer = Binarizer.BoolCast };
			GC.Collect(2, GCCollectionMode.Forced, true, true);
			using var barcode = Assert.Single(reader.Read(pointer, pixels.Length, pixels.Length, 1, ImageFormat.Lum));
			Assert.Equal("96385074", barcode.Text);

			Array.Fill(pixels, (byte)255);
			Marshal.Copy(pixels, 0, pointer, pixels.Length);
			Assert.Empty(reader.Read(pointer, pixels.Length, pixels.Length, 1, ImageFormat.Lum));
		} finally {
			Marshal.FreeHGlobal(pointer);
		}
	}

	[Fact]
	public void OptionalOptionsCanBeOmitted()
	{
		using var barcode = new Barcode("hello", BarcodeFormat.DataMatrix);
		using var image = barcode.ToImage();
		Assert.NotEmpty(image.ToArray());
		Assert.NotEmpty(barcode.ToSVG());
		var pixels = image.ToArray();
		var results = BarcodeReader.Read(pixels, image.Width, image.Height, image.Format);
		try {
			Assert.Equal("hello", Assert.Single(results).Text);
		} finally {
			foreach (var result in results)
				result.Dispose();
		}
	}

	[Fact]
	public void DisposedHandlesRejectFurtherUse()
	{
#pragma warning disable CS0618
		var view = new ImageView(new byte[1], 1, 1, ImageFormat.Lum);
		view.Dispose();
		view.Dispose();
		Assert.Throws<ObjectDisposedException>(() => BarcodeReader.Read(view));
#pragma warning restore CS0618

		var options = new ReaderOptions();
		options.Dispose();
		options.Dispose();
		Assert.Throws<ObjectDisposedException>(() => options.TryHarder);

		var barcode = new Barcode("hello", BarcodeFormat.DataMatrix);
		barcode.Dispose();
		barcode.Dispose();
		Assert.Throws<ObjectDisposedException>(() => barcode.Text);
	}

	[Fact]
	public void Read()
	{
		var data = Ean8Pixels();

		var br = new BarcodeReader {
			Binarizer = Binarizer.BoolCast,
		};
		var res = br.Read(data, data.Length, 1, ImageFormat.Lum);

		var expected = "96385074";

		Assert.Single(res);
		Assert.True(res[0].IsValid);
		Assert.Equal(BarcodeFormat.EAN8, res[0].Format);
		Assert.Equal(expected, res[0].Text);
		Assert.Equal(Encoding.ASCII.GetBytes(expected), res[0].Bytes);
		Assert.Equal("]E4", res[0].SymbologyIdentifier);
		Assert.False(res[0].HasECI);
		Assert.Equal(ContentType.Text, res[0].ContentType);
		Assert.Equal(0, res[0].Rotation);
		Assert.Equal(new PointI { X = 4, Y = 0 }, res[0].Position.TopLeft);
		Assert.Equal(1, res[0].LineCount);
		Assert.False(res[0].IsMirrored);
		Assert.False(res[0].IsInverted);
		Assert.Equal(ErrorType.None, res[0].ErrorType);
		Assert.Equal("", res[0].ErrorMsg);
		Assert.Equal(-1, res[0].SequenceIndex);
		Assert.Equal(-1, res[0].SequenceSize);
		Assert.Equal("", res[0].SequenceId);
		Assert.Equal("", res[0].Extra());
		Assert.Equal("", res[0].Extra("invalid_key"));
	}

	[Fact]
	public void Create()
	{
		var text = "hello";
		var res = new Barcode(text, BarcodeFormat.DataMatrix);

		Assert.True(res.IsValid);
		Assert.Equal(BarcodeFormat.DataMatrix, res.Format);
		Assert.Equal(text, res.Text);
		Assert.Equal(Encoding.ASCII.GetBytes(text), res.Bytes);
		Assert.False(res.HasECI);
		Assert.Equal(ContentType.Text, res.ContentType);
		Assert.Equal(0, res.Rotation);
		Assert.False(res.IsMirrored);
		Assert.False(res.IsInverted);
		Assert.Equal(new PointI { X = 0, Y = 0 }, res.Position.TopLeft);
		Assert.Equal(ErrorType.None, res.ErrorType);
		Assert.Equal("", res.ErrorMsg);
	}

	[Fact]
	public void Write()
	{
		var text = "hello";
		var res = new Barcode(text, BarcodeFormat.DataMatrix);

		var img = res.ToImage(new WriterOptions { Scale = 2, AddQuietZones = false });
		Assert.Equal(24, img.Width);
		Assert.Equal(24, img.Height);
	}
}
