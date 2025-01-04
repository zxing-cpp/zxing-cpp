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

using static Dll;
using BarcodeFormat = BarcodeFormats;

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
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setReturnErrors(IntPtr opts, bool returnErrors);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_ReaderOptions_getReturnErrors(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_ReaderOptions_setFormats(IntPtr opts, BarcodeFormats formats);
	[DllImport(DllName)] public static extern BarcodeFormats ZXing_ReaderOptions_getFormats(IntPtr opts);
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
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_BarcodeFormatFromString(string str);
	[DllImport(DllName)] public static extern BarcodeFormats ZXing_BarcodeFormatsFromString(string str);

	[DllImport(DllName)] public static extern IntPtr ZXing_ImageView_new(IntPtr data, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern IntPtr ZXing_ImageView_new_checked(byte[] data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride);
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
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setReaderInit(IntPtr opts, bool readerInit);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_CreatorOptions_getReaderInit(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setForceSquareDataMatrix(IntPtr opts, bool forceSquareDataMatrix);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_CreatorOptions_getForceSquareDataMatrix(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_CreatorOptions_setEcLevel(IntPtr opts, string ecLevel);
	[DllImport(DllName)] public static extern IntPtr ZXing_CreatorOptions_getEcLevel(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_WriterOptions_new();
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_delete(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setScale(IntPtr opts, int scale);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getScale(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setSizeHint(IntPtr opts, int sizeHint);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getSizeHint(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setRotate(IntPtr opts, int rotate);
	[DllImport(DllName)] public static extern int ZXing_WriterOptions_getRotate(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setWithHRT(IntPtr opts, bool withHRT);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getWithHRT(IntPtr opts);
	[DllImport(DllName)] public static extern void ZXing_WriterOptions_setWithQuietZones(IntPtr opts, bool withQuietZones);
	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_WriterOptions_getWithQuietZones(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_CreateBarcodeFromText(string data, int size, IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_CreateBarcodeFromBytes(byte[] data, int size, IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr ZXing_WriteBarcodeToSVG(IntPtr barcode, IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr ZXing_WriteBarcodeToImage(IntPtr barcode, IntPtr opts);

	[DllImport(DllName)] [return:MarshalAs(UnmanagedType.I1)] public static extern bool ZXing_Barcode_isValid(IntPtr barcode);
	[DllImport(DllName)] public static extern IntPtr ZXing_Barcode_errorMsg(IntPtr barcode);
	[DllImport(DllName)] public static extern ErrorType ZXing_Barcode_errorType(IntPtr barcode);
	[DllImport(DllName)] public static extern BarcodeFormat ZXing_Barcode_format(IntPtr barcode);
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
}

[Flags]
public enum BarcodeFormats
{
	None            = 0,         ///< Used as a return value if no valid barcode has been detected
	Aztec           = (1 << 0),  ///< Aztec
	Codabar         = (1 << 1),  ///< Codabar
	Code39          = (1 << 2),  ///< Code39
	Code93          = (1 << 3),  ///< Code93
	Code128         = (1 << 4),  ///< Code128
	DataBar         = (1 << 5),  ///< GS1 DataBar, formerly known as RSS 14
	DataBarExpanded = (1 << 6),  ///< GS1 DataBar Expanded, formerly known as RSS EXPANDED
	DataMatrix      = (1 << 7),  ///< DataMatrix
	EAN8            = (1 << 8),  ///< EAN-8
	EAN13           = (1 << 9),  ///< EAN-13
	ITF             = (1 << 10), ///< ITF (Interleaved Two of Five)
	MaxiCode        = (1 << 11), ///< MaxiCode
	PDF417          = (1 << 12), ///< PDF417
	QRCode          = (1 << 13), ///< QR Code
	UPCA            = (1 << 14), ///< UPC-A
	UPCE            = (1 << 15), ///< UPC-E
	MicroQRCode     = (1 << 16), ///< Micro QR Code
	RMQRCode        = (1 << 17), ///< Rectangular Micro QR Code
	DXFilmEdge      = (1 << 18), ///< DX Film Edge Barcode
	DataBarLimited  = (1 << 19), ///< GS1 DataBar Limited

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | DataBarLimited | DXFilmEdge | UPCA | UPCE,
	MatrixCodes = Aztec | DataMatrix | MaxiCode | PDF417 | QRCode | MicroQRCode | RMQRCode,
	Any         = LinearCodes | MatrixCodes,
};


public enum Binarizer
{
	LocalAverage,    ///< T = average of neighboring pixels for matrix and GlobalHistogram for linear (HybridBinarizer)
	GlobalHistogram, ///< T = valley between the 2 largest peaks in the histogram (per line in linear case)
	FixedThreshold,  ///< T = 127
	BoolCast,        ///< T = 0, fastest possible
};

public enum EanAddOnSymbol
{
	Ignore,  ///< Ignore any Add-On symbol during read/scan
	Read,    ///< Read EAN-2/EAN-5 Add-On symbol if found
	Require, ///< Require EAN-2/EAN-5 Add-On symbol to be present
};

public enum TextMode
{
	Plain,   ///< bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)
	ECI,     ///< standard content following the ECI protocol with every character set ECI segment transcoded to unicode
	HRI,     ///< Human Readable Interpretation (dependent on the ContentType)
	Hex,     ///< bytes() transcoded to ASCII string of HEX values
	Escaped, ///< Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to "<GS>")
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

public class ImageView
{
	internal IntPtr _d;

	public ImageView(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new_checked(data, data.Length, width, height, format, rowStride, pixStride));

	public ImageView(IntPtr data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
		=> _d = CheckError(ZXing_ImageView_new(data, width, height, format, rowStride, pixStride));

	~ImageView() => ZXing_ImageView_delete(_d);
}

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

public class ReaderOptions
{
	internal IntPtr _d;

	public ReaderOptions() => _d = CheckError(ZXing_ReaderOptions_new(), "Failed to create ReaderOptions.");

	~ReaderOptions() => ZXing_ReaderOptions_delete(_d);

	public bool TryHarder
	{
		get => ZXing_ReaderOptions_getTryHarder(_d);
		set => ZXing_ReaderOptions_setTryHarder(_d, value);
	}

	public bool TryRotate
	{
		get => ZXing_ReaderOptions_getTryRotate(_d);
		set => ZXing_ReaderOptions_setTryRotate(_d, value);
	}

	public bool TryInvert
	{
		get => ZXing_ReaderOptions_getTryInvert(_d);
		set => ZXing_ReaderOptions_setTryInvert(_d, value);
	}

	public bool TryDownscale
	{
		get => ZXing_ReaderOptions_getTryDownscale(_d);
		set => ZXing_ReaderOptions_setTryDownscale(_d, value);
	}

	public bool IsPure
	{
		get => ZXing_ReaderOptions_getIsPure(_d);
		set => ZXing_ReaderOptions_setIsPure(_d, value);
	}

	public bool ReturnErrors
	{
		get => ZXing_ReaderOptions_getReturnErrors(_d);
		set => ZXing_ReaderOptions_setReturnErrors(_d, value);
	}

	public BarcodeFormats Formats
	{
		get => ZXing_ReaderOptions_getFormats(_d);
		set => ZXing_ReaderOptions_setFormats(_d, value);
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

	public int MinLineCount
	{
		get => ZXing_ReaderOptions_getMinLineCount(_d);
		set => ZXing_ReaderOptions_setMinLineCount(_d, value);
	}

	public int MaxNumberOfSymbols
	{
		get => ZXing_ReaderOptions_getMaxNumberOfSymbols(_d);
		set => ZXing_ReaderOptions_setMaxNumberOfSymbols(_d, value);
	}

}

public class CreatorOptions
{
	internal IntPtr _d;

	public CreatorOptions(BarcodeFormat format)
		=> _d = CheckError(ZXing_CreatorOptions_new(format), "Failed to create CreatorOptions.");

	public static implicit operator CreatorOptions(BarcodeFormat f) => new CreatorOptions(f);

	~CreatorOptions() => ZXing_CreatorOptions_delete(_d);

	public bool ReaderInit
	{
		get => ZXing_CreatorOptions_getReaderInit(_d);
		set => ZXing_CreatorOptions_setReaderInit(_d, value);
	}

	public bool ForceSquareDataMatrix
	{
		get => ZXing_CreatorOptions_getForceSquareDataMatrix(_d);
		set => ZXing_CreatorOptions_setForceSquareDataMatrix(_d, value);
	}

	public String ECLevel
	{
		get => MarshalAsString(ZXing_CreatorOptions_getEcLevel(_d));
		set => ZXing_CreatorOptions_setEcLevel(_d, value);
	}

}

public class WriterOptions
{
	internal IntPtr _d;

	public WriterOptions() => _d = CheckError(ZXing_WriterOptions_new(), "Failed to create WriterOptions.");

	~WriterOptions() => ZXing_WriterOptions_delete(_d);

	public int Scale
	{
		get => ZXing_WriterOptions_getScale(_d);
		set => ZXing_WriterOptions_setScale(_d, value);
	}

	public int SizeHint
	{
		get => ZXing_WriterOptions_getSizeHint(_d);
		set => ZXing_WriterOptions_setSizeHint(_d, value);
	}

	public int Rotate
	{
		get => ZXing_WriterOptions_getRotate(_d);
		set => ZXing_WriterOptions_setRotate(_d, value);
	}

	public bool WithHRT
	{
		get => ZXing_WriterOptions_getWithHRT(_d);
		set => ZXing_WriterOptions_setWithHRT(_d, value);
	}

	public bool WithQuietZones
	{
		get => ZXing_WriterOptions_getWithQuietZones(_d);
		set => ZXing_WriterOptions_setWithQuietZones(_d, value);
	}
}

public class Barcode
{
	internal IntPtr _d;

	internal Barcode(IntPtr d) => _d = d;

	~Barcode() => ZXing_Barcode_delete(_d);

	public Barcode(string data, CreatorOptions opts)
		=> _d = CheckError(ZXing_CreateBarcodeFromText(data, data.Length, opts._d));

	public Barcode(byte[] data, CreatorOptions opts)
		=> _d = CheckError(ZXing_CreateBarcodeFromBytes(data, data.Length, opts._d));

	public bool IsValid => ZXing_Barcode_isValid(_d);
	public BarcodeFormat Format => ZXing_Barcode_format(_d);
	public ContentType ContentType => ZXing_Barcode_contentType(_d);
	public string Text => MarshalAsString(ZXing_Barcode_text(_d));
	public byte[] Bytes => MarshalAsBytes(ZXing_Barcode_bytes, _d);
	public byte[] BytesECI => MarshalAsBytes(ZXing_Barcode_bytesECI, _d);
	public string ECLevel => MarshalAsString(ZXing_Barcode_ecLevel(_d));
	public string SymbologyIdentifier => MarshalAsString(ZXing_Barcode_symbologyIdentifier(_d));
	public string ErrorMsg => MarshalAsString(ZXing_Barcode_errorMsg(_d));
	public ErrorType ErrorType => ZXing_Barcode_errorType(_d);
	public Position Position => ZXing_Barcode_position(_d);
	public int Orientation => ZXing_Barcode_orientation(_d);
	public bool HasECI => ZXing_Barcode_hasECI(_d);
	public bool IsInverted => ZXing_Barcode_isInverted(_d);
	public bool IsMirrored => ZXing_Barcode_isMirrored(_d);
	public int LineCount => ZXing_Barcode_lineCount(_d);

	public string ToSVG(WriterOptions? opts = null)
		=> MarshalAsString(CheckError(ZXing_WriteBarcodeToSVG(_d, opts?._d ?? IntPtr.Zero)));

	public Image ToImage(WriterOptions? opts = null)
		=> new Image(CheckError(ZXing_WriteBarcodeToImage(_d, opts?._d ?? IntPtr.Zero)));

	public static BarcodeFormat FormatFromString(string str)
	{
		var res = ZXing_BarcodeFormatFromString(str);
		if ((int)res == -1) // see ZXing_BarcodeFormat_Invalid
			throw new Exception(MarshalAsString(ZXing_LastErrorMsg()));
		return res;
	}

	public static BarcodeFormats FormatsFromString(string str)
	{
		var res = ZXing_BarcodeFormatsFromString(str);
		if ((int)res == -1) // see ZXing_BarcodeFormat_Invalid
			throw new Exception(MarshalAsString(ZXing_LastErrorMsg()));
		return res;
	}
}

public class BarcodeReader : ReaderOptions
{
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

public class BarcodeCreator : CreatorOptions
{
	public BarcodeCreator(BarcodeFormat format) : base(format) {}

	public Barcode From(string data) => new Barcode(data, this);
	public Barcode From(byte[] data) => new Barcode(data, this);
}

}
