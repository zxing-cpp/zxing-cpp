using Xunit;
using System.Text;
using ZXingCpp;

namespace ZXingCpp.Tests;

public class UnitTest1
{
	[Fact]
	public void ValidBarcodeFormatsParsing()
	{
		Assert.Equal(BarcodeFormats.QRCode, BarcodeReader.FormatsFromString("qrcode"));
		Assert.Equal(BarcodeFormats.LinearCodes, BarcodeReader.FormatsFromString("linear_codes"));
		Assert.Equal(BarcodeFormats.None, BarcodeReader.FormatsFromString(""));
	}

	[Fact]
	public void InvalidBarcodeFormatsParsing()
	{
		Assert.Throws<Exception>(() => BarcodeReader.FormatsFromString("nope"));
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
		var res = br.Read(iv);

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
	}
}