using Xunit;
using System.Text;
using ZXingCpp;

namespace ZXingCpp.Tests;

public class UnitTest1
{
	[Fact]
	public void ValidBarcodeFormatsParsing()
	{
		Assert.Equal(BarcodeFormats.QRCode, Barcode.FormatsFromString("qrcode"));
		Assert.Equal(BarcodeFormats.LinearCodes, Barcode.FormatsFromString("linear_codes"));
		Assert.Equal(BarcodeFormats.None, Barcode.FormatsFromString(""));
	}

	[Fact]
	public void InvalidBarcodeFormatsParsing()
	{
		Assert.Throws<Exception>(() => Barcode.FormatsFromString("nope"));
	}

	[Fact]
	public void InvalidImageView()
	{
		Assert.Throws<Exception>(() => new ImageView(new byte[0], 1, 1, ImageFormat.Lum));
		Assert.Throws<Exception>(() => new ImageView(new byte[1], 1, 1, ImageFormat.Lum, 2));
	}

	[Fact]
	public void Read()
	{
		var data = new List<byte>();
		foreach (var v in "0000101000101101011110111101011011101010100111011100101000100101110010100000")
			data.Add((byte)(v == '0' ? 255 : 0));

		var iv = new ImageView(data.ToArray(), data.Count, 1, ImageFormat.Lum);
		var br = new BarcodeReader() {
			Binarizer = Binarizer.BoolCast,
		};
		var res = br.From(iv);

		var expected = "96385074";

		Assert.Single(res);
		Assert.True(res[0].IsValid);
		Assert.Equal(BarcodeFormats.EAN8, res[0].Format);
		Assert.Equal(expected, res[0].Text);
		Assert.Equal(Encoding.ASCII.GetBytes(expected), res[0].Bytes);
		Assert.False(res[0].HasECI);
		Assert.Equal(ContentType.Text, res[0].ContentType);
		Assert.Equal(0, res[0].Orientation);
		Assert.Equal(new PointI() { X = 4, Y = 0 }, res[0].Position.TopLeft);
		Assert.Equal(1, res[0].LineCount);
		Assert.False(res[0].IsMirrored);
		Assert.False(res[0].IsInverted);
		Assert.Equal(ErrorType.None, res[0].ErrorType);
		Assert.Equal("", res[0].ErrorMsg);
	}

	[Fact]
	public void Create()
	{
		var text = "hello";
		var res = new Barcode(text, BarcodeFormats.DataMatrix);

		Assert.True(res.IsValid);
		Assert.Equal(BarcodeFormats.DataMatrix, res.Format);
		Assert.Equal(text, res.Text);
		Assert.Equal(Encoding.ASCII.GetBytes(text), res.Bytes);
		Assert.False(res.HasECI);
		Assert.Equal(ContentType.Text, res.ContentType);
		Assert.Equal(0, res.Orientation);
		Assert.False(res.IsMirrored);
		Assert.False(res.IsInverted);
		Assert.Equal(new PointI() { X = 1, Y = 1 }, res.Position.TopLeft);
		Assert.Equal(ErrorType.None, res.ErrorType);
		Assert.Equal("", res.ErrorMsg);
	}
}