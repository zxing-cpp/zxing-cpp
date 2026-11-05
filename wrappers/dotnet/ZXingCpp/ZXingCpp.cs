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

internal abstract class NativeHandle : SafeHandle
{
	internal static readonly NativeHandle Null = new NullHandle();
	protected NativeHandle(bool ownsHandle = true) : base(IntPtr.Zero, ownsHandle) {}
	public override bool IsInvalid => handle == IntPtr.Zero;
}

internal sealed class NullHandle : NativeHandle
{
	internal NullHandle() : base(false) {}
	protected override bool ReleaseHandle() => true;
}

internal sealed class ReaderOptionsHandle : NativeHandle
{
	public ReaderOptionsHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_ReaderOptions_delete(handle); return true; }
}

internal sealed class CreatorOptionsHandle : NativeHandle
{
	public CreatorOptionsHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_CreatorOptions_delete(handle); return true; }
}

internal sealed class WriterOptionsHandle : NativeHandle
{
	public WriterOptionsHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_WriterOptions_delete(handle); return true; }
}

internal sealed class ImageViewHandle : NativeHandle
{
	public ImageViewHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_ImageView_delete(handle); return true; }
}

internal sealed class ImageHandle : NativeHandle
{
	public ImageHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_Image_delete(handle); return true; }
}

internal sealed class BarcodeHandle : NativeHandle
{
	public BarcodeHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_Barcode_delete(handle); return true; }
}

internal sealed class BarcodesHandle : NativeHandle
{
	public BarcodesHandle() {}
	protected override bool ReleaseHandle() { Dll.ZXing_Barcodes_delete(handle); return true; }
}

internal class Dll
{
	private const string DllName = "ZXing";

	[DllImport(DllName)] public static extern ReaderOptionsHandle ZXing_ReaderOptions_new();
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_delete(IntPtr opts);

	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryHarder(ReaderOptionsHandle opts, bool tryHarder);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryHarder(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryRotate(ReaderOptionsHandle opts, bool tryRotate);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryRotate(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryInvert(ReaderOptionsHandle opts, bool tryInvert);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryInvert(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryDownscale(ReaderOptionsHandle opts, bool tryDownscale);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryDownscale(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTryDenoise(ReaderOptionsHandle opts, bool tryDenoise);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getTryDenoise(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setIsPure(ReaderOptionsHandle opts, bool isPure);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getIsPure(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setValidateOptionalChecksum(ReaderOptionsHandle opts, bool validateOptionalChecksum);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getValidateOptionalChecksum(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setReturnErrors(ReaderOptionsHandle opts, bool returnErrors);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getReturnErrors(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setFormats(ReaderOptionsHandle opts, BarcodeFormat[] formats, int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_ReaderOptions_getFormats(ReaderOptionsHandle opts, out int count);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setBinarizer(ReaderOptionsHandle opts, Binarizer binarizer);
	[DllImport(DllName)] public static extern Binarizer ZXing_ReaderOptions_getBinarizer(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setEanAddOnSymbol(ReaderOptionsHandle opts, EanAddOnSymbol eanAddOnSymbol);
	[DllImport(DllName)] public static extern EanAddOnSymbol ZXing_ReaderOptions_getEanAddOnSymbol(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setTextMode(ReaderOptionsHandle opts, TextMode textMode);
	[DllImport(DllName)] public static extern TextMode ZXing_ReaderOptions_getTextMode(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setMinLineCount(ReaderOptionsHandle opts, int n);
	[DllImport(DllName)] public static extern int ZXing_ReaderOptions_getMinLineCount(ReaderOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setMaxNumberOfSymbols(ReaderOptionsHandle opts, int n);
	[DllImport(DllName)] public static extern int ZXing_ReaderOptions_getMaxNumberOfSymbols(ReaderOptionsHandle opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_PositionToString(Position position);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_BarcodeFormatSymbology(BarcodeFormat format);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_BarcodeFormatFromString(string str);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatToString(BarcodeFormat format);

	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsList(BarcodeFormat filter, out int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsFromString(string str, out int count);
	[DllImport(DllName)] public static extern IntPtr ZXing_BarcodeFormatsToString(BarcodeFormat[] format, int count);

	[DllImport(DllName)] public static extern ImageViewHandle ZXing_ImageView_new(IntPtr data, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern ImageViewHandle ZXing_ImageView_new_checked(IntPtr data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride);
	//[DllImport(DllName)] public static extern ImageViewHandle ZXing_ImageView_new_checked(ref byte data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern void ZXing_ImageView_delete(IntPtr iv);

	[DllImport(DllName)] public static extern void ZXing_Image_delete(IntPtr img);
	[DllImport(DllName)] public static extern IntPtr ZXing_Image_data(ImageHandle img);
	[DllImport(DllName)] public static extern int ZXing_Image_width(ImageHandle img);
	[DllImport(DllName)] public static extern int ZXing_Image_height(ImageHandle img);
	[DllImport(DllName)] public static extern ImageFormat ZXing_Image_format(ImageHandle img);

	[DllImport(DllName)] public static extern BarcodesHandle ZXing_ReadBarcodes(ImageViewHandle iv, NativeHandle opts);
	[DllImport(DllName)] public static extern void ZXing_Barcode_delete(IntPtr barcode);
	[DllImport(DllName)] public static extern void ZXing_Barcodes_delete(IntPtr barcodes);
	[DllImport(DllName)] public static extern int ZXing_Barcodes_size(BarcodesHandle barcodes);
	[DllImport(DllName)] public static extern BarcodeHandle ZXing_Barcodes_move(BarcodesHandle barcodes, int i);

	[DllImport(DllName)] public static extern CreatorOptionsHandle ZXing_CreatorOptions_new(BarcodeFormat format);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_delete(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setFormat(CreatorOptionsHandle opts, BarcodeFormat format);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_CreatorOptions_getFormat(CreatorOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setOptions(CreatorOptionsHandle opts, string options);
	[DllImport(DllName)] public static extern IntPtr ZXing_CreatorOptions_getOptions(CreatorOptionsHandle opts);

	[DllImport(DllName)] public static extern WriterOptionsHandle ZXing_WriterOptions_new();
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_delete(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setScale(WriterOptionsHandle opts, int scale);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getScale(WriterOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setRotate(WriterOptionsHandle opts, int rotate);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getRotate(WriterOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setAddHRT(WriterOptionsHandle opts, bool addHRT);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getAddHRT(WriterOptionsHandle opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setAddQuietZones(WriterOptionsHandle opts, bool addQuietZones);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getAddQuietZones(WriterOptionsHandle opts);

	[DllImport(DllName)] public static extern BarcodeHandle ZXing_CreateBarcodeFromText(string data, int size, CreatorOptionsHandle opts);
	[DllImport(DllName)] public static extern BarcodeHandle ZXing_CreateBarcodeFromBytes(byte[] data, int size, CreatorOptionsHandle opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_WriteBarcodeToSVG(BarcodeHandle barcode, NativeHandle opts);
	[DllImport(DllName)] public static extern ImageHandle ZXing_WriteBarcodeToImage(BarcodeHandle barcode, NativeHandle opts);

	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isValid(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_errorMsg(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern ErrorType ZXing_Barcode_errorType(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_Barcode_format(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_Barcode_symbology(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern ContentType ZXing_Barcode_contentType(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_bytes(BarcodeHandle barcode, out int len);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_bytesECI(BarcodeHandle barcode, out int len);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_text(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_symbologyIdentifier(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern Position ZXing_Barcode_position(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_rotation(BarcodeHandle barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_hasECI(BarcodeHandle barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isInverted(BarcodeHandle barcode);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isMirrored(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_lineCount(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_sequenceIndex(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern int ZXing_Barcode_sequenceSize(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_sequenceId(BarcodeHandle barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_extra(BarcodeHandle barcode, string? key);

	[DllImport(DllName)] public static extern void ZXing_free(IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_LastErrorMsg();


	public static IntPtr CheckError(IntPtr ptr, string? msg = null)
	{
		if (ptr == IntPtr.Zero)
			throw new Exception(msg ?? MarshalAsString(ZXing_LastErrorMsg()));
		return ptr;
	}

	public static T CheckError<T>(T handle, string? msg = null) where T : SafeHandle
	{
		if (handle.IsInvalid)
			throw new Exception(msg ?? MarshalAsString(ZXing_LastErrorMsg()));
		return handle;
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

	public delegate IntPtr RetBytesFunc(BarcodeHandle ptr, out int len);

	public static byte[] MarshalAsBytes(RetBytesFunc func, BarcodeHandle d)
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

	public static readonly BarcodeFormat Invalid         = new BarcodeFormat(0xFFFF);
	public static readonly BarcodeFormat None            = new BarcodeFormat(0x0000);
	public static readonly BarcodeFormat All             = new BarcodeFormat(0x2A2A);
	public static readonly BarcodeFormat AllReadable     = new BarcodeFormat(0x722A);
	public static readonly BarcodeFormat AllCreatable    = new BarcodeFormat(0x772A);
	public static readonly BarcodeFormat AllLinear       = new BarcodeFormat(0x6C2A);
	public static readonly BarcodeFormat AllMatrix       = new BarcodeFormat(0x6D2A);
	public static readonly BarcodeFormat AllGS1          = new BarcodeFormat(0x472A);
	public static readonly BarcodeFormat AllRetail       = new BarcodeFormat(0x522A);
	public static readonly BarcodeFormat AllIndustrial   = new BarcodeFormat(0x492A);
	public static readonly BarcodeFormat Codabar         = new BarcodeFormat(0x2046);
	public static readonly BarcodeFormat Code39          = new BarcodeFormat(0x2041);
	public static readonly BarcodeFormat Code39Std       = new BarcodeFormat(0x7341);
	public static readonly BarcodeFormat Code39Ext       = new BarcodeFormat(0x6541);
	public static readonly BarcodeFormat Code32          = new BarcodeFormat(0x3241);
	public static readonly BarcodeFormat PZN             = new BarcodeFormat(0x7041);
	public static readonly BarcodeFormat Code93          = new BarcodeFormat(0x2047);
	public static readonly BarcodeFormat Code128         = new BarcodeFormat(0x2043);
	public static readonly BarcodeFormat ITF             = new BarcodeFormat(0x2049);
	public static readonly BarcodeFormat ITF14           = new BarcodeFormat(0x3449);
	public static readonly BarcodeFormat DataBar         = new BarcodeFormat(0x2065);
	public static readonly BarcodeFormat DataBarOmni     = new BarcodeFormat(0x6F65);
	public static readonly BarcodeFormat DataBarStk      = new BarcodeFormat(0x7365);
	public static readonly BarcodeFormat DataBarStkOmni  = new BarcodeFormat(0x4F65);
	public static readonly BarcodeFormat DataBarLtd      = new BarcodeFormat(0x6C65);
	public static readonly BarcodeFormat DataBarExp      = new BarcodeFormat(0x6565);
	public static readonly BarcodeFormat DataBarExpStk   = new BarcodeFormat(0x4565);
	public static readonly BarcodeFormat EANUPC          = new BarcodeFormat(0x2045);
	public static readonly BarcodeFormat EAN13           = new BarcodeFormat(0x3145);
	public static readonly BarcodeFormat EAN8            = new BarcodeFormat(0x3845);
	public static readonly BarcodeFormat EAN5            = new BarcodeFormat(0x3545);
	public static readonly BarcodeFormat EAN2            = new BarcodeFormat(0x3245);
	public static readonly BarcodeFormat ISBN            = new BarcodeFormat(0x6945);
	public static readonly BarcodeFormat UPCA            = new BarcodeFormat(0x6145);
	public static readonly BarcodeFormat UPCE            = new BarcodeFormat(0x6545);
	public static readonly BarcodeFormat Telepen         = new BarcodeFormat(0x2042);
	public static readonly BarcodeFormat TelepenAlpha    = new BarcodeFormat(0x3042);
	public static readonly BarcodeFormat TelepenNumeric  = new BarcodeFormat(0x3142);
	public static readonly BarcodeFormat OtherBarcode    = new BarcodeFormat(0x2058);
	public static readonly BarcodeFormat DXFilmEdge      = new BarcodeFormat(0x7858);
	public static readonly BarcodeFormat PDF417          = new BarcodeFormat(0x204C);
	public static readonly BarcodeFormat CompactPDF417   = new BarcodeFormat(0x634C);
	public static readonly BarcodeFormat MicroPDF417     = new BarcodeFormat(0x6D4C);
	public static readonly BarcodeFormat Aztec           = new BarcodeFormat(0x207A);
	public static readonly BarcodeFormat AztecCode       = new BarcodeFormat(0x637A);
	public static readonly BarcodeFormat AztecRune       = new BarcodeFormat(0x727A);
	public static readonly BarcodeFormat QRCode          = new BarcodeFormat(0x2051);
	public static readonly BarcodeFormat QRCodeModel1    = new BarcodeFormat(0x3151);
	public static readonly BarcodeFormat QRCodeModel2    = new BarcodeFormat(0x3251);
	public static readonly BarcodeFormat MicroQRCode     = new BarcodeFormat(0x6D51);
	public static readonly BarcodeFormat RMQRCode        = new BarcodeFormat(0x7251);
	public static readonly BarcodeFormat DataMatrix      = new BarcodeFormat(0x2064);
	public static readonly BarcodeFormat MaxiCode        = new BarcodeFormat(0x2055);

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
	/// <exception cref="Exception">Thrown when the string is not a valid format name.</exception>
	public static BarcodeFormat Parse(string s)
	{
		if (!TryParse(s, out var fmt))
			throw new Exception($"Invalid BarcodeFormat: '{s}'");
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
/// Obsolete native image view. Prefer the buffer overloads on <see cref="BarcodeReader"/>.
/// </summary>
[Obsolete("Use BarcodeReader.Read overloads that take image data directly.")]
public class ImageView : IDisposable
{
	internal readonly ImageViewHandle _d;
	private GCHandle _pin;

	/// <summary>Creates a native view over the array. The array remains pinned until this view is disposed.</summary>
	public ImageView(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
	{
		if (data == null)
			throw new ArgumentNullException(nameof(data));
		_pin = GCHandle.Alloc(data, GCHandleType.Pinned);
		try {
			_d = CheckError(ZXing_ImageView_new_checked(_pin.AddrOfPinnedObject(), data.Length, width, height, format, rowStride, pixStride));
		} catch {
			_pin.Free();
			throw;
		}
	}

#if NET
	/// <summary>Creates a non-owning view. The caller must keep the span's memory pinned and alive until this view is disposed.</summary>
	public unsafe ImageView(ReadOnlySpan<byte> data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
	{
		fixed (byte* pixels = data)
			_d = CheckError(ZXing_ImageView_new_checked((IntPtr)pixels, data.Length, width, height, format, rowStride, pixStride));
	}
#endif

	/// <summary>Creates a non-owning view. The caller must keep the buffer valid until this view is disposed.</summary>
	public ImageView(IntPtr data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new(data, width, height, format, rowStride, pixStride));

	~ImageView() => Dispose(false);

	public void Dispose()
	{
		Dispose(true);
		GC.SuppressFinalize(this);
	}

	private void Dispose(bool disposing)
	{
		lock (this) {
			_d?.Dispose();
			if (_pin.IsAllocated)
				_pin.Free();
		}
	}
}

/// <summary>
/// Owns barcode image data. Returned by Barcode.ToImage().
/// </summary>
public class Image : IDisposable
{
	internal readonly ImageHandle _d;

	internal Image(ImageHandle d) => _d = d;

	public void Dispose() => _d.Dispose();

	/// <summary>Borrowed pixel pointer, valid only while this image remains alive and undisposed.</summary>
	public IntPtr Data => ZXing_Image_data(_d);
	public int Width => ZXing_Image_width(_d);
	public int Height => ZXing_Image_height(_d);
	public ImageFormat Format => ZXing_Image_format(_d);

	public byte[] ToArray()
	{
		bool addedRef = false;
		try {
			_d.DangerousAddRef(ref addedRef);
			IntPtr ptr = ZXing_Image_data(_d);
			if (ptr == IntPtr.Zero)
				return Array.Empty<byte>();

			int len = Width * Height;
			byte[] res = new byte[len];
			Marshal.Copy(ptr, res, 0, len);
			return res;
		} finally {
			if (addedRef)
				_d.DangerousRelease();
		}
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
	internal readonly ReaderOptionsHandle _d;

	public ReaderOptions() => _d = CheckError(ZXing_ReaderOptions_new(), "Failed to create ReaderOptions.");

	public void Dispose() => _d.Dispose();

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

	/// <summary>Also try detection after denoising.</summary>
	public bool TryDenoise
	{
		get => ZXing_ReaderOptions_getTryDenoise(_d);
		set => ZXing_ReaderOptions_setTryDenoise(_d, value);
	}

	/// <summary>Assume the image contains only a single, perfectly aligned barcode, nothing else.</summary>
	public bool IsPure
	{
		get => ZXing_ReaderOptions_getIsPure(_d);
		set => ZXing_ReaderOptions_setIsPure(_d, value);
	}

	/// <summary>Validate optional checksums (e.g., Code39, ITF).</summary>
	public bool ValidateOptionalChecksum
	{
		get => ZXing_ReaderOptions_getValidateOptionalChecksum(_d);
		set => ZXing_ReaderOptions_setValidateOptionalChecksum(_d, value);
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
	internal readonly CreatorOptionsHandle _d;

	public CreatorOptions(BarcodeFormat format, string? options = null)
	{
		_d = CheckError(ZXing_CreatorOptions_new(format), "Failed to create CreatorOptions.");
		if (options != null)
			Options = options;
	}

	public static implicit operator CreatorOptions(BarcodeFormat f) => new CreatorOptions(f);

	public void Dispose() => _d.Dispose();

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
	internal readonly WriterOptionsHandle _d;

	public WriterOptions() => _d = CheckError(ZXing_WriterOptions_new(), "Failed to create WriterOptions.");

	public void Dispose() => _d.Dispose();

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
	internal readonly BarcodeHandle _d;

	internal Barcode(BarcodeHandle d) => _d = d;

	public void Dispose() => _d.Dispose();

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
	/// <summary>ISO/IEC 15424 symbology identifier (e.g., "]Q1" for QR Code).</summary>
	public string SymbologyIdentifier => MarshalAsString(ZXing_Barcode_symbologyIdentifier(_d));
	public string ErrorMsg => MarshalAsString(ZXing_Barcode_errorMsg(_d));
	public ErrorType ErrorType => ZXing_Barcode_errorType(_d);
	/// <summary>Corner points of the barcode in the image.</summary>
	public Position Position => ZXing_Barcode_position(_d);
	/// <summary>Detected rotation in degrees.</summary>
	public int Rotation => ZXing_Barcode_rotation(_d);
	[Obsolete("Use Rotation instead")]
	public int Orientation => Rotation;
	public bool HasECI => ZXing_Barcode_hasECI(_d);
	public bool IsInverted => ZXing_Barcode_isInverted(_d);
	public bool IsMirrored => ZXing_Barcode_isMirrored(_d);
	public int LineCount => ZXing_Barcode_lineCount(_d);
	public int SequenceIndex => ZXing_Barcode_sequenceIndex(_d);
	public int SequenceSize => ZXing_Barcode_sequenceSize(_d);
	public string SequenceId => MarshalAsString(ZXing_Barcode_sequenceId(_d));
	/// <summary>
	/// Additional format-specific metadata as JSON pairs (e.g., '{"EcLevel":"H","Version":5}' for QR Codes).
	/// </summary>
	/// <param name="key"></param>
	/// <returns>JSON string or value of the specified key in the extra metadata.</returns>
	public string Extra(string? key = null) => MarshalAsString(ZXing_Barcode_extra(_d, key));

	public string ToSVG(WriterOptions? opts = null)
		=> MarshalAsString(CheckError(ZXing_WriteBarcodeToSVG(_d, (NativeHandle?)opts?._d ?? NativeHandle.Null)));

	public Image ToImage(WriterOptions? opts = null)
		=> new Image(CheckError(ZXing_WriteBarcodeToImage(_d, (NativeHandle?)opts?._d ?? NativeHandle.Null)));
}

/// <summary>Barcode reader. Inherits ReaderOptions for convenient configuration.</summary>
public class BarcodeReader : ReaderOptions
{
	private static Barcode[] ReadNative(ImageViewHandle view, ReaderOptions? opts)
	{
		using var ptr = CheckError(ZXing_ReadBarcodes(view, (NativeHandle?)opts?._d ?? NativeHandle.Null));

		var size = ZXing_Barcodes_size(ptr);
		if (size == 0)
			return Array.Empty<Barcode>();
		var res = new Barcode[size];
		try {
			for (int i = 0; i < size; ++i)
				res[i] = new Barcode(CheckError(ZXing_Barcodes_move(ptr, i)));
			return res;
		} catch {
			foreach (var barcode in res)
				barcode?.Dispose();
			throw;
		}
	}

	private static Barcode[] ReadPinned(IntPtr data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride, ReaderOptions? opts)
	{
		using var view = CheckError(ZXing_ImageView_new_checked(data, size, width, height, format, rowStride, pixStride));
		return ReadNative(view, opts);
	}

	/// <summary>Scans an image for barcodes.</summary>
	/// <param name="iv">Image to scan.</param>
	/// <param name="opts">Optional configuration; uses defaults if null.</param>
	/// <returns>Array of detected barcodes (empty if none found).</returns>
	[Obsolete("Use a Read overload that takes image data directly.")]
	public static Barcode[] Read(ImageView iv, ReaderOptions? opts = null)
	{
		if (iv == null)
			throw new ArgumentNullException(nameof(iv));
		lock (iv)
			return ReadNative(iv._d, opts);
	}

	/// <summary>Scans a byte array synchronously without copying it. Do not modify the pixels during the scan.</summary>
	public static Barcode[] Read(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0, ReaderOptions? opts = null)
	{
		if (data == null)
			throw new ArgumentNullException(nameof(data));
		var pin = GCHandle.Alloc(data, GCHandleType.Pinned);
		try {
			return ReadPinned(pin.AddrOfPinnedObject(), data.Length, width, height, format, rowStride, pixStride, opts);
		} finally {
			pin.Free();
		}
	}

	/// <summary>Scans caller-owned memory synchronously without copying it.</summary>
	/// <param name="data">Pointer to the first image byte.</param>
	/// <param name="size">Available bytes from <paramref name="data"/>.</param>
	/// <remarks>The caller must keep the buffer valid and unchanged until this call returns.</remarks>
	public static Barcode[] Read(IntPtr data, int size, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0, ReaderOptions? opts = null)
		=> ReadPinned(data, size, width, height, format, rowStride, pixStride, opts);

#if NET
	/// <summary>Scans a span synchronously without copying it. Do not modify the pixels during the scan.</summary>
	public static unsafe Barcode[] Read(ReadOnlySpan<byte> data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0, ReaderOptions? opts = null)
	{
		fixed (byte* pixels = data) {
			return ReadPinned((IntPtr)pixels, data.Length, width, height, format, rowStride, pixStride, opts);
		}
	}
#endif

	[Obsolete("Use a Read overload that takes image data directly.")]
	public Barcode[] From(ImageView iv) => Read(iv, this);
	public Barcode[] Read(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> Read(data, width, height, format, rowStride, pixStride, this);
	public Barcode[] Read(IntPtr data, int size, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> Read(data, size, width, height, format, rowStride, pixStride, this);
#if NET
	public Barcode[] Read(ReadOnlySpan<byte> data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> Read(data, width, height, format, rowStride, pixStride, this);
#endif
}

/// <summary>Barcode creator. Inherits CreatorOptions for convenient configuration.</summary>
public class BarcodeCreator : CreatorOptions
{
	public BarcodeCreator(BarcodeFormat format) : base(format) {}

	public Barcode From(string data) => new Barcode(data, this);
	public Barcode From(byte[] data) => new Barcode(data, this);
}

}
