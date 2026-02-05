/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

namespace ZXingCpp {

using System;
#if NETSTANDARD
using System.Text;
#endif
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;

using static Dll;

internal class Dll
{
	private const string DllName = "ZXing";

	[DllImport(DllName)] public static extern IntPtr ZXing_ReaderOptions_new();
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_delete(IntPtr opts);

	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryHarder(IntPtr opts, bool tryHarder);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryHarder(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryRotate(IntPtr opts, bool tryRotate);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryRotate(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryInvert(IntPtr opts, bool tryInvert);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryInvert(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryDownscale(IntPtr opts, bool tryDownscale);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryDownscale(IntPtr opts);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getIsPure(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setIsPure(IntPtr opts, bool isPure);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setValidateOptionalCheckSum(IntPtr opts, bool validateOptionalCheckSum);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getValidateOptionalCheckSum(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setReturnErrors(IntPtr opts, bool returnErrors);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getReturnErrors(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setFormats(IntPtr opts, BarcodeFormat[] formats, int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_ReaderOptions_getFormats(IntPtr opts, out int count);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setBinarizer(IntPtr opts, Binarizer binarizer);
	[DllImport(DllName)] public static extern Binarizer ZXing_ReaderOptions_getBinarizer(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setEanAddOnSymbol(IntPtr opts, EanAddOnSymbol eanAddOnSymbol);
	[DllImport(DllName)] public static extern EanAddOnSymbol ZXing_ReaderOptions_getEanAddOnSymbol(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTextMode(IntPtr opts, TextMode textMode);
	[DllImport(DllName)] public static extern TextMode ZXing_ReaderOptions_getTextMode(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setMinLineCount(IntPtr opts, int n);
	[DllImport(DllName)] public static extern int ZXing_ReaderOptions_getMinLineCount(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setMaxNumberOfSymbols(IntPtr opts, int n);
	[DllImport(DllName)] public static extern int ZXing_ReaderOptions_getMaxNumberOfSymbols(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_PositionToString(Position position);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_BarcodeFormatSymbology(BarcodeFormat format);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_BarcodeFormatFromString(string str);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatToString(BarcodeFormat format);

	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsList(BarcodeFormat filter, out int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsFromString(string str, out int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsToString(BarcodeFormat[] format, int count);

	[DllImport(DllName)] public static extern IntPtr ZXing_ImageView_new(IntPtr data, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern IntPtr ZXing_ImageView_new_checked(ref byte data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern void ZXing_ImageView_delete(IntPtr iv);

	[DllImport(DllName)] public static extern void ZXing_Image_delete(IntPtr img);
	[DllImport(DllName)] public static extern IntPtr ZXing_Image_data(IntPtr img);
	[DllImport(DllName)] public static extern int ZXing_Image_width(IntPtr img);
	[DllImport(DllName)] public static extern int ZXing_Image_height(IntPtr img);
	[DllImport(DllName)] public static extern ImageFormat ZXing_Image_format(IntPtr img);

	[DllImport(DllName)] public static extern IntPtr ZXing_ReadBarcodes(IntPtr iv, IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_Barcode_delete(IntPtr barcode);
	[DllImport(DllName)] public static extern void ZXing_Barcodes_delete(IntPtr barcodes);
	[DllImport(DllName)] public static extern int ZXing_Barcodes_size(IntPtr barcodes);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcodes_move(IntPtr barcodes, int i);

	[DllImport(DllName)] public static extern IntPtr ZXing_CreatorOptions_new(BarcodeFormat format);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_delete(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setFormat(IntPtr opts, BarcodeFormat format);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_CreatorOptions_getFormat(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setOptions(IntPtr opts, string options);
	[DllImport(DllName)] public static extern IntPtr ZXing_CreatorOptions_getOptions(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_WriterOptions_new();
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_delete(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setScale(IntPtr opts, int scale);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getScale(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setRotate(IntPtr opts, int rotate);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getRotate(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setAddHRT(IntPtr opts, bool addHRT);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getAddHRT(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setAddQuietZones(IntPtr opts, bool addQuietZones);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getAddQuietZones(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_CreateBarcodeFromText(string data, int size, IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_CreateBarcodeFromBytes(byte[] data, int size, IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_WriteBarcodeToSVG(IntPtr barcode, IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_WriteBarcodeToImage(IntPtr barcode, IntPtr opts);

	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isValid(IntPtr barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_errorMsg(IntPtr barcode);
	[DllImport(DllName)] public static extern ErrorType ZXing_Barcode_errorType(IntPtr barcode);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_Barcode_format(IntPtr barcode);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_Barcode_symbology(IntPtr barcode);
	[DllImport(DllName)] public static extern ContentType ZXing_Barcode_contentType(IntPtr barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_bytes(IntPtr barcode, out int len);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_bytesECI(IntPtr barcode, out int len);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_text(IntPtr barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_ecLevel(IntPtr barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_symbologyIdentifier(IntPtr barcode);
	[DllImport(DllName)] public static extern Position ZXing_Barcode_position(IntPtr barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_orientation(IntPtr barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_hasECI(IntPtr barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isInverted(IntPtr barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isMirrored(IntPtr barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_lineCount(IntPtr barcode);

	[DllImport(DllName)] public static extern void ZXing_free(IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_LastErrorMsg();


	public static IntPtr CheckError(IntPtr ptr, string? msg = null)
	{
		if (ptr == IntPtr.Zero)
			throw new Exception(msg ?? MarshalAsString(ZXing_LastErrorMsg()));
		return ptr;
	}

	public static string MarshalAsString(IntPtr ptr)
	{
		ptr = CheckError(ptr, "ZXing C-API returned a NULL char*.");
#if NET
		string res = Marshal.PtrToStringUTF8(ptr) ?? "";
#else
		string res;
		unsafe
		{
			int length = 0;
			for (byte* i = (byte*)ptr; *i != 0; i++, length++);
			res = Encoding.UTF8.GetString((byte*)ptr, length);
		}
#endif
		ZXing_free(ptr);
		return res;
	}

	public delegate IntPtr RetBytesFunc(IntPtr ptr, out int len);

	public static byte[] MarshalAsBytes(RetBytesFunc func, IntPtr d)
	{
		IntPtr ptr = CheckError(func(d, out int len), "ZXing C-API returned a NULL byte*.");
		byte[] res = new byte[len];
		Marshal.Copy(ptr, res, 0, len);
		ZXing_free(ptr);
		return res;
	}

	public static BarcodeFormat[] MarshalAsFormats(IntPtr ptr, int count)
	{
		if (ptr == IntPtr.Zero || count <= 0)
			return Array.Empty<BarcodeFormat>();

		int[] vals = new int[count];
		Marshal.Copy(ptr, vals, 0, count);
		var res = Array.ConvertAll(vals, v => (BarcodeFormat)v);
		ZXing_free(ptr);
		return res;
	}
}

/// <summary>
/// Represents a barcode format/symbology. Can represent a specific format (e.g., QRCode)
/// or a combination of formats (e.g., AllReadable).
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 1, Size = 4)]
public readonly struct BarcodeFormat : IEquatable<BarcodeFormat>
{
	private readonly int _value;
	private BarcodeFormat(int v) => _value = v;

	public static readonly BarcodeFormat Invalid = new BarcodeFormat(0xFFFF);
	public static readonly BarcodeFormat None = new BarcodeFormat(0x0000);
	public static readonly BarcodeFormat AllReadable = new BarcodeFormat(0x722A);
	public static readonly BarcodeFormat AllCreatable = new BarcodeFormat(0x772A);
	public static readonly BarcodeFormat AllLinear = new BarcodeFormat(0x6CAA);
	public static readonly BarcodeFormat AllMatrix = new BarcodeFormat(0x6D8A);
	public static readonly BarcodeFormat AllGS1 = new BarcodeFormat(0x670A);
	public static readonly BarcodeFormat Codabar = new BarcodeFormat(0x2046);
	public static readonly BarcodeFormat Code39 = new BarcodeFormat(0x2041);
	public static readonly BarcodeFormat PZN = new BarcodeFormat(0x7041);
	public static readonly BarcodeFormat Code93 = new BarcodeFormat(0x2047);
	public static readonly BarcodeFormat Code128 = new BarcodeFormat(0x2043);
	public static readonly BarcodeFormat ITF = new BarcodeFormat(0x2049);
	public static readonly BarcodeFormat DataBar = new BarcodeFormat(0x2065);
	public static readonly BarcodeFormat DataBarOmni = new BarcodeFormat(0x6F65);
	public static readonly BarcodeFormat DataBarLtd = new BarcodeFormat(0x6C65);
	public static readonly BarcodeFormat DataBarExp = new BarcodeFormat(0x6565);
	public static readonly BarcodeFormat EANUPC = new BarcodeFormat(0x2045);
	public static readonly BarcodeFormat EAN13 = new BarcodeFormat(0x3145);
	public static readonly BarcodeFormat EAN8 = new BarcodeFormat(0x3845);
	public static readonly BarcodeFormat EAN5 = new BarcodeFormat(0x3545);
	public static readonly BarcodeFormat EAN2 = new BarcodeFormat(0x3245);
	public static readonly BarcodeFormat ISBN = new BarcodeFormat(0x6945);
	public static readonly BarcodeFormat UPCA = new BarcodeFormat(0x6145);
	public static readonly BarcodeFormat UPCE = new BarcodeFormat(0x6545);
	public static readonly BarcodeFormat DXFilmEdge = new BarcodeFormat(0x7858);
	public static readonly BarcodeFormat PDF417 = new BarcodeFormat(0x204C);
	public static readonly BarcodeFormat CompactPDF417 = new BarcodeFormat(0x634C);
	public static readonly BarcodeFormat MicroPDF417 = new BarcodeFormat(0x6D4C);
	public static readonly BarcodeFormat Aztec = new BarcodeFormat(0x207A);
	public static readonly BarcodeFormat AztecCode = new BarcodeFormat(0x637A);
	public static readonly BarcodeFormat AztecRune = new BarcodeFormat(0x727A);
	public static readonly BarcodeFormat QRCode = new BarcodeFormat(0x2051);
	public static readonly BarcodeFormat QRCodeModel1 = new BarcodeFormat(0x3151);
	public static readonly BarcodeFormat MicroQRCode = new BarcodeFormat(0x6D51);
	public static readonly BarcodeFormat RMQRCode = new BarcodeFormat(0x7251);
	public static readonly BarcodeFormat DataMatrix = new BarcodeFormat(0x2064);
	public static readonly BarcodeFormat MaxiCode = new BarcodeFormat(0x2055);

	public static implicit operator int(BarcodeFormat f) => f._value;
	public static implicit operator BarcodeFormat(int v) => new BarcodeFormat(v);

	public bool Equals(BarcodeFormat other) => _value == other._value;
	public override bool Equals(object? obj) => obj is BarcodeFormat other && Equals(other);
	public override int GetHashCode() => _value.GetHashCode();
	public static bool operator ==(BarcodeFormat a, BarcodeFormat b) => a.Equals(b);
	public static bool operator !=(BarcodeFormat a, BarcodeFormat b) => !a.Equals(b);

	/// <summary>Converts the barcode format to its human readable string representation.</summary>
	/// <returns>The format name (e.g., "QR Code", "DataBar Limited").</returns>
	public override string ToString() => MarshalAsString(ZXing_BarcodeFormatToString(this));

	/// <summary>Parses a string into a BarcodeFormat.</summary>
	/// <param name="s">Format name (case-insensitive).</param>
	/// <exception cref="FormatException">Thrown when the string is not a valid format name.</exception>
	public static BarcodeFormat Parse(string s)
	{
		if (!TryParse(s, out var fmt))
			throw new FormatException($"Invalid BarcodeFormat: '{s}'");
		return fmt;
	}

	/// <summary>Attempts to parse a string into a BarcodeFormat.</summary>
	/// <returns>true if parsing succeeded; otherwise false.</returns>
	public static bool TryParse(string? s, out BarcodeFormat format)
	{
		format = ZXing_BarcodeFormatFromString(s ?? "");
		return format != Invalid;
	}

	/// <summary>Gets the symbology for this format (e.g., EAN13 returns EANUPC).</summary>
	public BarcodeFormat Symbology() => ZXing_BarcodeFormatSymbology(this);
}

/// <summary>Binarization algorithm for converting grayscale images to black/white for detection.</summary>
public enum Binarizer
{
	/// <summary>Average of neighboring pixels for matrix, GlobalHistogram for linear (HybridBinarizer).</summary>
	LocalAverage,
	/// <summary>Valley between the 2 largest peaks in the histogram (per line in linear case).</summary>
	GlobalHistogram,
	/// <summary>Fixed threshold at 127.</summary>
	FixedThreshold,
	/// <summary>Threshold at 0, fastest option.</summary>
	BoolCast,
};

/// <summary>Handling of EAN-2/EAN-5 Add-On symbols.</summary>
public enum EanAddOnSymbol
{
	/// <summary>Ignore any Add-On symbol during read/scan.</summary>
	Ignore,
	/// <summary>Read EAN-2/EAN-5 Add-On symbol if found.</summary>
	Read,
	/// <summary>Require EAN-2/EAN-5 Add-On symbol to be present.</summary>
	Require,
};

/// <summary>Text encoding mode that controls how barcode content bytes are converted to strings.</summary>
public enum TextMode
{
	/// <summary>bytes() transcoded to unicode based on ECI info or guessed charset (default mode prior to 2.0).</summary>
	Plain,
	/// <summary>Standard content following the ECI protocol with every character set ECI segment transcoded to unicode.</summary>
	ECI,
	/// <summary>Human Readable Interpretation (dependent on the ContentType).</summary>
	HRI,
	/// <summary>Use EscapeNonGraphical() function (e.g., ASCII 29 becomes "&lt;GS&gt;").</summary>
	Escaped,
	/// <summary>bytes() transcoded to ASCII string of HEX values.</summary>
	Hex,
	/// <summary>bytesECI() transcoded to ASCII string of HEX values.</summary>
	HexECI,
};

public enum ContentType { Text, Binary, Mixed, GS1, ISO15434, UnknownECI };

public enum ErrorType { None, Format, Checksum, Unsupported };

public enum ImageFormat {
	None = 0,
	Lum = 0x01000000,
	LumA = 0x02000000,
	RGB = 0x03000102,
	BGR = 0x03020100,
	RGBA = 0x04000102,
	ARGB = 0x04010203,
	BGRA = 0x04020100,
	ABGR = 0x04030201,
};

public struct PointI
{
	public int X, Y;
};

public struct Position
{
	public PointI TopLeft, TopRight, BottomRight, BottomLeft;

	public override string ToString() => MarshalAsString(ZXing_PositionToString(this));
};

/// <summary>
/// Non-owning view into image data for barcode detection. Does not copy pixel data.
/// </summary>
public class ImageView
{
	internal IntPtr _d;

#if NET
	public ImageView(ReadOnlySpan<byte> data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new_checked(ref MemoryMarshal.GetReference(data), data.Length, width, height, format, rowStride, pixStride));
#else
	public ImageView(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new_checked(ref data[0], data.Length, width, height, format, rowStride, pixStride));
#endif

	public ImageView(IntPtr data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new(data, width, height, format, rowStride, pixStride));

	~ImageView() => ZXing_ImageView_delete(_d);
}

/// <summary>
/// Owns barcode image data. Returned by Barcode.ToImage().
/// </summary>
public class Image : IDisposable
{
	internal IntPtr _d;

	internal Image(IntPtr d) => _d = d;

	~Image() => Dispose();

	public void Dispose()
	{
		ZXing_Image_delete(_d);
		_d = IntPtr.Zero;
		GC.SuppressFinalize(this);
	}

	public IntPtr Data => ZXing_Image_data(_d);
	public int Width => ZXing_Image_width(_d);
	public int Height => ZXing_Image_height(_d);
	public ImageFormat Format => ZXing_Image_format(_d);

	public byte[] ToArray()
	{
		IntPtr ptr = ZXing_Image_data(_d);
		if (ptr == IntPtr.Zero)
			return new byte[0];

		int len = Width * Height;
		byte[] res = new byte[len];
		Marshal.Copy(ptr, res, 0, len);
		return res;
	}
}

/// <summary>
/// Collection of barcode formats, supporting combinations like "QRCode, DataMatrix".
/// </summary>
public class BarcodeFormats : IReadOnlyCollection<BarcodeFormat>
{
	private readonly BarcodeFormat[] _d;

	public BarcodeFormats() => _d = Array.Empty<BarcodeFormat>();
	public BarcodeFormats(params BarcodeFormat[] formats) => _d = formats ?? Array.Empty<BarcodeFormat>();
	public BarcodeFormats(IEnumerable<BarcodeFormat> formats) => _d = formats?.ToArray() ?? Array.Empty<BarcodeFormat>();

	public static implicit operator BarcodeFormats(BarcodeFormat format) => new BarcodeFormats(format);
	public static implicit operator BarcodeFormat[](BarcodeFormats formats) => formats._d;

	public bool Contains(BarcodeFormat format) => Array.IndexOf(_d, format) >= 0;
	public int Count => _d.Length;
	public bool IsEmpty => _d.Length == 0;

	public IEnumerator<BarcodeFormat> GetEnumerator() => ((IEnumerable<BarcodeFormat>)_d).GetEnumerator();
	System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator() => _d.GetEnumerator();

	/// <summary>Lists all supported barcode formats, optionally filtered by a specific format.</summary>
	/// <param name="filter">e.g. AllReadable or AllLinear.</param>
	/// <returns>All supported barcode formats that match the filter.</returns>
	public static BarcodeFormats List(BarcodeFormat filter)
	{
		IntPtr ptr = ZXing_BarcodeFormatsList(filter, out int count);
		return new BarcodeFormats(MarshalAsFormats(ptr, count));
	}

	public static bool TryParse(string str, out BarcodeFormats formats)
	{
		IntPtr ptr = ZXing_BarcodeFormatsFromString(str, out int count);
		formats = new BarcodeFormats(MarshalAsFormats(ptr, count));
		return formats.Count > 0;
	}

	public static BarcodeFormats Parse(string str)
	{
		if (TryParse(str, out var formats))
			return formats;

		throw new Exception($"'{str}' is not a valid set of barcode formats.");
	}

	public override string ToString() => MarshalAsString(ZXing_BarcodeFormatsToString(_d, _d.Length));
}

/// <summary>Configuration options for barcode reading/detection.</summary>
public class ReaderOptions : IDisposable
{
	internal IntPtr _d;

	public ReaderOptions() => _d = CheckError(ZXing_ReaderOptions_new(), "Failed to create ReaderOptions.");

	~ReaderOptions() => Dispose();

	public void Dispose()
	{
		ZXing_ReaderOptions_delete(_d);
		_d = IntPtr.Zero;
		GC.SuppressFinalize(this);
	}

	/// <summary>Spend more time to find barcodes; slower but more accurate.</summary>
	public bool TryHarder
	{
		get => ZXing_ReaderOptions_getTryHarder(_d);
		set => ZXing_ReaderOptions_setTryHarder(_d, value);
	}

	/// <summary>Also detect barcodes in 90/180/270 degree rotated images.</summary>
	public bool TryRotate
	{
		get => ZXing_ReaderOptions_getTryRotate(_d);
		set => ZXing_ReaderOptions_setTryRotate(_d, value);
	}

	/// <summary>Also try detecting inverted (white on black) barcodes.</summary>
	public bool TryInvert
	{
		get => ZXing_ReaderOptions_getTryInvert(_d);
		set => ZXing_ReaderOptions_setTryInvert(_d, value);
	}

	/// <summary>Try downscaled images (high resolution images can hamper the detection).</summary>
	public bool TryDownscale
	{
		get => ZXing_ReaderOptions_getTryDownscale(_d);
		set => ZXing_ReaderOptions_setTryDownscale(_d, value);
	}

	/// <summary>Assume the image contains only a single, perfectly aligned barcode, nothing else.</summary>
	public bool IsPure
	{
		get => ZXing_ReaderOptions_getIsPure(_d);
		set => ZXing_ReaderOptions_setIsPure(_d, value);
	}

	/// <summary>Validate optional checksums (e.g., Code39, ITF).</summary>
	public bool ValidateOptionalCheckSum
	{
		get => ZXing_ReaderOptions_getValidateOptionalCheckSum(_d);
		set => ZXing_ReaderOptions_setValidateOptionalCheckSum(_d, value);
	}

	/// <summary>Return invalid barcodes with error information instead of skipping them.</summary>
	public bool ReturnErrors
	{
		get => ZXing_ReaderOptions_getReturnErrors(_d);
		set => ZXing_ReaderOptions_setReturnErrors(_d, value);
	}

	/// <summary>Barcode formats to search for (increase performance by limiting the set).</summary>
	public BarcodeFormats Formats
	{
		get {
			IntPtr ptr = ZXing_ReaderOptions_getFormats(_d, out int count);
			return new BarcodeFormats(MarshalAsFormats(ptr, count));
		}
		set {
			ZXing_ReaderOptions_setFormats(_d, value, value.Count);
		}
	}

	public Binarizer Binarizer
	{
		get => ZXing_ReaderOptions_getBinarizer(_d);
		set => ZXing_ReaderOptions_setBinarizer(_d, value);
	}

	public EanAddOnSymbol EanAddOnSymbol
	{
		get => ZXing_ReaderOptions_getEanAddOnSymbol(_d);
		set => ZXing_ReaderOptions_setEanAddOnSymbol(_d, value);
	}

	public TextMode TextMode
	{
		get => ZXing_ReaderOptions_getTextMode(_d);
		set => ZXing_ReaderOptions_setTextMode(_d, value);
	}

	/// <summary>Minimum number of lines for linear barcodes (default is 2).</summary>
	public int MinLineCount
	{
		get => ZXing_ReaderOptions_getMinLineCount(_d);
		set => ZXing_ReaderOptions_setMinLineCount(_d, value);
	}

	/// <summary>Maximum number of symbols to detect (can be used to limit processing time).</summary>
	public int MaxNumberOfSymbols
	{
		get => ZXing_ReaderOptions_getMaxNumberOfSymbols(_d);
		set => ZXing_ReaderOptions_setMaxNumberOfSymbols(_d, value);
	}

}

/// <summary>Options for creating/encoding barcodes.</summary>
public class CreatorOptions : IDisposable
{
	internal IntPtr _d;

	public CreatorOptions(BarcodeFormat format, string? options = null)
	{
		_d = CheckError(ZXing_CreatorOptions_new(format), "Failed to create CreatorOptions.");
		if (options != null)
			Options = options;
	}

	public static implicit operator CreatorOptions(BarcodeFormat f) => new CreatorOptions(f);

	~CreatorOptions() => Dispose();

	public void Dispose()
	{
		ZXing_CreatorOptions_delete(_d);
		_d = IntPtr.Zero;
		GC.SuppressFinalize(this);
	}

	public BarcodeFormat Format
	{
		get => ZXing_CreatorOptions_getFormat(_d);
		set => ZXing_CreatorOptions_setFormat(_d, value);
	}

	/// <summary>Format-specific options as key=value pairs (e.g., "EcLevel=H").</summary>
	/// <remarks>This can be a serialized JSON object or simple key=value pairs separated by commas.</remarks>
	public String Options
	{
		get => MarshalAsString(ZXing_CreatorOptions_getOptions(_d));
		set => ZXing_CreatorOptions_setOptions(_d, value);
	}

}

/// <summary>Options for rendering barcodes to images or SVG.</summary>
public class WriterOptions : IDisposable
{
	internal IntPtr _d;

	public WriterOptions() => _d = CheckError(ZXing_WriterOptions_new(), "Failed to create WriterOptions.");

	~WriterOptions() => Dispose();

	public void Dispose()
	{
		ZXing_WriterOptions_delete(_d);
		_d = IntPtr.Zero;
		GC.SuppressFinalize(this);
	}

	/// <summary>Scaling factor for the barcode modules (>0 means 'pixels per module', <0 means 'target size in pixels').</summary>
	public int Scale
	{
		get => ZXing_WriterOptions_getScale(_d);
		set => ZXing_WriterOptions_setScale(_d, value);
	}

	/// <summary>Rotation in degrees (0, 90, 180, or 270).</summary>
	public int Rotate
	{
		get => ZXing_WriterOptions_getRotate(_d);
		set => ZXing_WriterOptions_setRotate(_d, value);
	}

	/// <summary>Add Human Readable Text (the barcode content) below linear barcodes.</summary>
	public bool AddHRT
	{
		get => ZXing_WriterOptions_getAddHRT(_d);
		set => ZXing_WriterOptions_setAddHRT(_d, value);
	}

	/// <summary>Add standard quiet zones (white margins) around the barcode.</summary>
	public bool AddQuietZones
	{
		get => ZXing_WriterOptions_getAddQuietZones(_d);
		set => ZXing_WriterOptions_setAddQuietZones(_d, value);
	}
}

/// <summary>A detected or created barcode with its data, format, and metadata.</summary>
public class Barcode : IDisposable
{
	internal IntPtr _d;

	internal Barcode(IntPtr d) => _d = d;

	~Barcode() => Dispose();

	public void Dispose()
	{
		ZXing_Barcode_delete(_d);
		_d = IntPtr.Zero;
		GC.SuppressFinalize(this);
	}

	public Barcode(string data, CreatorOptions opts)
		=> _d = CheckError(ZXing_CreateBarcodeFromText(data, data.Length, opts._d));

	public Barcode(byte[] data, CreatorOptions opts)
		=> _d = CheckError(ZXing_CreateBarcodeFromBytes(data, data.Length, opts._d));

	/// <summary>True if successfully decoded or created without errors.</summary>
	public bool IsValid => ZXing_Barcode_isValid(_d);
	public BarcodeFormat Format => ZXing_Barcode_format(_d);
	/// <summary>Base symbology (e.g., EAN13 -> EANUPC).</summary>
	public BarcodeFormat Symbology => ZXing_Barcode_symbology(_d);
	public ContentType ContentType => ZXing_Barcode_contentType(_d);
	public string Text => MarshalAsString(ZXing_Barcode_text(_d));
	public byte[] Bytes => MarshalAsBytes(ZXing_Barcode_bytes, _d);
	/// <summary>Bytes with Extended Channel Interpretation markers included.</summary>
	public byte[] BytesECI => MarshalAsBytes(ZXing_Barcode_bytesECI, _d);
	/// <summary>Error correction level (e.g., "H" for QR codes).</summary>
	public string ECLevel => MarshalAsString(ZXing_Barcode_ecLevel(_d));
	/// <summary>ISO/IEC 15424 symbology identifier (e.g., "]Q1" for QR Code).</summary>
	public string SymbologyIdentifier => MarshalAsString(ZXing_Barcode_symbologyIdentifier(_d));
	public string ErrorMsg => MarshalAsString(ZXing_Barcode_errorMsg(_d));
	public ErrorType ErrorType => ZXing_Barcode_errorType(_d);
	/// <summary>Corner points of the barcode in the image.</summary>
	public Position Position => ZXing_Barcode_position(_d);
	/// <summary>Detected rotation in degrees.</summary>
	public int Orientation => ZXing_Barcode_orientation(_d);
	public bool HasECI => ZXing_Barcode_hasECI(_d);
	public bool IsInverted => ZXing_Barcode_isInverted(_d);
	public bool IsMirrored => ZXing_Barcode_isMirrored(_d);
	public int LineCount => ZXing_Barcode_lineCount(_d);

	public string ToSVG(WriterOptions? opts = null)
		=> MarshalAsString(CheckError(ZXing_WriteBarcodeToSVG(_d, opts?._d ?? IntPtr.Zero)));

	public Image ToImage(WriterOptions? opts = null)
		=> new Image(CheckError(ZXing_WriteBarcodeToImage(_d, opts?._d ?? IntPtr.Zero)));
}

/// <summary>Barcode reader. Inherits ReaderOptions for convenient configuration.</summary>
public class BarcodeReader : ReaderOptions
{
	/// <summary>Scans an image for barcodes.</summary>
	/// <param name="iv">Image to scan.</param>
	/// <param name="opts">Optional configuration; uses defaults if null.</param>
	/// <returns>Array of detected barcodes (empty if none found).</returns>
	public static Barcode[] Read(ImageView iv, ReaderOptions? opts = null)
	{
		var ptr = CheckError(ZXing_ReadBarcodes(iv._d, opts?._d ?? IntPtr.Zero));

		// return static empty array if no Barcodes are found, avoiding any managed heap allocation
		var res = Array.Empty<Barcode>();
		var size = ZXing_Barcodes_size(ptr);
		if (size > 0) {
			res = new Barcode[size];
			for (int i = 0; i < size; ++i)
				res[i] = new Barcode(ZXing_Barcodes_move(ptr, i));
		}

		ZXing_Barcodes_delete(ptr);

		return res;
	}

	public Barcode[] From(ImageView iv) => Read(iv, this);
}

/// <summary>Barcode creator. Inherits CreatorOptions for convenient configuration.</summary>
public class BarcodeCreator : CreatorOptions
{
	public BarcodeCreator(BarcodeFormat format) : base(format) {}

	public Barcode From(string data) => new Barcode(data, this);
	public Barcode From(byte[] data) => new Barcode(data, this);
}

}
