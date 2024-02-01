/*
* Copyright 2024 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

namespace ZXingCpp {

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using static Dll;
using BarcodeFormat = BarcodeFormats;

internal class Dll
{
	private const string DllName = "ZXing";

	[DllImport(DllName)] public static extern IntPtr zxing_ReaderOptions_new();
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_delete(IntPtr opts);

	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setTryHarder(IntPtr opts, bool tryHarder);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getTryHarder(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setTryRotate(IntPtr opts, bool tryRotate);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getTryRotate(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setTryInvert(IntPtr opts, bool tryInvert);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getTryInvert(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setTryDownscale(IntPtr opts, bool tryDownscale);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getTryDownscale(IntPtr opts);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getIsPure(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setIsPure(IntPtr opts, bool isPure);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setReturnErrors(IntPtr opts, bool returnErrors);
	[DllImport(DllName)] public static extern bool zxing_ReaderOptions_getReturnErrors(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setFormats(IntPtr opts, BarcodeFormats formats);
	[DllImport(DllName)] public static extern BarcodeFormats zxing_ReaderOptions_getFormats(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setBinarizer(IntPtr opts, Binarizer binarizer);
	[DllImport(DllName)] public static extern Binarizer zxing_ReaderOptions_getBinarizer(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setEanAddOnSymbol(IntPtr opts, EanAddOnSymbol eanAddOnSymbol);
	[DllImport(DllName)] public static extern EanAddOnSymbol zxing_ReaderOptions_getEanAddOnSymbol(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setTextMode(IntPtr opts, TextMode textMode);
	[DllImport(DllName)] public static extern TextMode zxing_ReaderOptions_getTextMode(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setMinLineCount(IntPtr opts, int n);
	[DllImport(DllName)] public static extern int zxing_ReaderOptions_getMinLineCount(IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_ReaderOptions_setMaxNumberOfSymbols(IntPtr opts, int n);
	[DllImport(DllName)] public static extern int zxing_ReaderOptions_getMaxNumberOfSymbols(IntPtr opts);

	[DllImport(DllName)] public static extern IntPtr zxing_PositionToString(Position position);
	[DllImport(DllName)] public static extern BarcodeFormats zxing_BarcodeFormatsFromString(string str);

	[DllImport(DllName)] public static extern IntPtr zxing_ImageView_new(IntPtr data, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern IntPtr zxing_ImageView_new_checked(byte[] data, int size, int width, int height, ImageFormat format, int rowStride, int pixStride);
	[DllImport(DllName)] public static extern void zxing_ImageView_delete(IntPtr iv);

	[DllImport(DllName)] public static extern IntPtr zxing_ReadBarcodes(IntPtr iv, IntPtr opts);
	[DllImport(DllName)] public static extern void zxing_Barcode_delete(IntPtr result);
	[DllImport(DllName)] public static extern void zxing_Barcodes_delete(IntPtr results);
	[DllImport(DllName)] public static extern int zxing_Barcodes_size(IntPtr results);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcodes_move(IntPtr results, int i);

	[DllImport(DllName)] public static extern bool zxing_Barcode_isValid(IntPtr result);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_errorMsg(IntPtr result);
	[DllImport(DllName)] public static extern BarcodeFormat zxing_Barcode_format(IntPtr result);
	[DllImport(DllName)] public static extern ContentType zxing_Barcode_contentType(IntPtr result);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_bytes(IntPtr result, out int len);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_bytesECI(IntPtr result, out int len);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_text(IntPtr result);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_ecLevel(IntPtr result);
	[DllImport(DllName)] public static extern IntPtr zxing_Barcode_symbologyIdentifier(IntPtr result);
	[DllImport(DllName)] public static extern Position zxing_Barcode_position(IntPtr result);
	[DllImport(DllName)] public static extern int zxing_Barcode_orientation(IntPtr result);
	[DllImport(DllName)] public static extern bool zxing_Barcode_hasECI(IntPtr result);
	[DllImport(DllName)] public static extern bool zxing_Barcode_isInverted(IntPtr result);
	[DllImport(DllName)] public static extern bool zxing_Barcode_isMirrored(IntPtr result);
	[DllImport(DllName)] public static extern int zxing_Barcode_lineCount(IntPtr result);

	[DllImport(DllName)] public static extern void zxing_free(IntPtr opts);
	[DllImport(DllName)] public static extern IntPtr zxing_LastErrorMsg();


	public static string MarshalAsString(IntPtr ptr)
	{
		if (ptr == IntPtr.Zero)
			throw new Exception("ZXing C-API returned a NULL char*.");

		string res = Marshal.PtrToStringUTF8(ptr) ?? "";
		zxing_free(ptr);
		return res;
	}

	public delegate IntPtr RetBytesFunc(IntPtr ptr, out int len);

	public static byte[] MarshalAsBytes(RetBytesFunc func, IntPtr d)
	{
		IntPtr ptr = func(d, out int len);
		if (ptr == IntPtr.Zero)
			throw new Exception("ZXing C-API returned a NULL byte*.");

		byte[] res = new byte[len];
		Marshal.Copy(ptr, res, 0, len);
		zxing_free(ptr);
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

	LinearCodes = Codabar | Code39 | Code93 | Code128 | EAN8 | EAN13 | ITF | DataBar | DataBarExpanded | DXFilmEdge | UPCA | UPCE,
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

public enum ImageFormat {
	None = 0,
	Lum = 0x01000000,
	RGB = 0x03000102,
	BGR = 0x03020100,
	RGBX = 0x04000102,
	XRGB = 0x04010203,
	BGRX = 0x04020100,
	XBGR = 0x04030201,
};

public struct PointI
{
	public int X, Y;
};

public struct Position
{
	public PointI TopLeft, TopRight, BottomRight, BottomLeft;

	public override string ToString() => MarshalAsString(zxing_PositionToString(this));
};

public class ImageView
{
	internal IntPtr _d;

	public ImageView(byte[] data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
	{
		_d = zxing_ImageView_new_checked(data, data.Length, width, height, format, rowStride, pixStride);
		if (_d == IntPtr.Zero)
			throw new Exception(MarshalAsString(zxing_LastErrorMsg()));
	}

	public ImageView(IntPtr data, int width, int height, ImageFormat format, int rowStride = 0, int pixStride = 0)
	{
		_d = zxing_ImageView_new(data, width, height, format, rowStride, pixStride);
		if (_d == IntPtr.Zero)
			throw new Exception(MarshalAsString(zxing_LastErrorMsg()));
	}

	~ImageView() => zxing_ImageView_delete(_d);
}

public class ReaderOptions
{
	internal IntPtr _d;

	public ReaderOptions()
	{
		_d = zxing_ReaderOptions_new();
		if (_d == IntPtr.Zero)
			throw new Exception("Failed to create ReaderOptions.");
	}

	~ReaderOptions() => zxing_ReaderOptions_delete(_d);

	public bool TryHarder
	{
		get => zxing_ReaderOptions_getTryHarder(_d);
		set => zxing_ReaderOptions_setTryHarder(_d, value);
	}

	public bool TryRotate
	{
		get => zxing_ReaderOptions_getTryRotate(_d);
		set => zxing_ReaderOptions_setTryRotate(_d, value);
	}

	public bool TryInvert
	{
		get => zxing_ReaderOptions_getTryInvert(_d);
		set => zxing_ReaderOptions_setTryInvert(_d, value);
	}

	public bool TryDownscale
	{
		get => zxing_ReaderOptions_getTryDownscale(_d);
		set => zxing_ReaderOptions_setTryDownscale(_d, value);
	}

	public bool IsPure
	{
		get => zxing_ReaderOptions_getIsPure(_d);
		set => zxing_ReaderOptions_setIsPure(_d, value);
	}

	public bool ReturnErrors
	{
		get => zxing_ReaderOptions_getReturnErrors(_d);
		set => zxing_ReaderOptions_setReturnErrors(_d, value);
	}

	public BarcodeFormats Formats
	{
		get => zxing_ReaderOptions_getFormats(_d);
		set => zxing_ReaderOptions_setFormats(_d, value);
	}

	public Binarizer Binarizer
	{
		get => zxing_ReaderOptions_getBinarizer(_d);
		set => zxing_ReaderOptions_setBinarizer(_d, value);
	}

	public EanAddOnSymbol EanAddOnSymbol
	{
		get => zxing_ReaderOptions_getEanAddOnSymbol(_d);
		set => zxing_ReaderOptions_setEanAddOnSymbol(_d, value);
	}

	public TextMode TextMode
	{
		get => zxing_ReaderOptions_getTextMode(_d);
		set => zxing_ReaderOptions_setTextMode(_d, value);
	}

	public int MinLineCount
	{
		get => zxing_ReaderOptions_getMinLineCount(_d);
		set => zxing_ReaderOptions_setMinLineCount(_d, value);
	}

	public int MaxNumberOfSymbols
	{
		get => zxing_ReaderOptions_getMaxNumberOfSymbols(_d);
		set => zxing_ReaderOptions_setMaxNumberOfSymbols(_d, value);
	}

}

public class Barcode
{
	internal IntPtr _d;

	internal Barcode(IntPtr d) => _d = d;
	~Barcode() => zxing_Barcode_delete(_d);

	public bool IsValid => zxing_Barcode_isValid(_d);
	public BarcodeFormat Format => zxing_Barcode_format(_d);
	public ContentType ContentType => zxing_Barcode_contentType(_d);
	public string Text => MarshalAsString(zxing_Barcode_text(_d));
	public byte[] Bytes => MarshalAsBytes(zxing_Barcode_bytes, _d);
	public byte[] BytesECI => MarshalAsBytes(zxing_Barcode_bytesECI, _d);
	public string ECLevel => MarshalAsString(zxing_Barcode_ecLevel(_d));
	public string SymbologyIdentifier => MarshalAsString(zxing_Barcode_symbologyIdentifier(_d));
	public string ErrorMsg => MarshalAsString(zxing_Barcode_errorMsg(_d));
	public Position Position => zxing_Barcode_position(_d);
	public int Orientation => zxing_Barcode_orientation(_d);
	public bool HasECI => zxing_Barcode_hasECI(_d);
	public bool IsInverted => zxing_Barcode_isInverted(_d);
	public bool IsMirrored => zxing_Barcode_isMirrored(_d);
	public int LineCount => zxing_Barcode_lineCount(_d);
}

public class BarcodeReader : ReaderOptions
{
	public static BarcodeFormats FormatsFromString(string str)
	{
		var fmts = zxing_BarcodeFormatsFromString(str);
		if ((int)fmts == -1) // see zxing_BarcodeFormat_Invalid
			throw new Exception(MarshalAsString(zxing_LastErrorMsg()));
		return fmts;
	}

	public static List<Barcode> Read(ImageView iv, ReaderOptions? opts = null)
	{
		var ptr = zxing_ReadBarcodes(iv._d, opts?._d ?? IntPtr.Zero);
		if (ptr == IntPtr.Zero)
			throw new Exception(MarshalAsString(zxing_LastErrorMsg()));

		var size = zxing_Barcodes_size(ptr);
		var res = new List<Barcode>(size);
		for (int i = 0; i < size; ++i)
			res.Add(new Barcode(zxing_Barcodes_move(ptr, i)));
		zxing_Barcodes_delete(ptr);

		return res;
	}

	public List<Barcode> Read(ImageView iv) => Read(iv, this);
}

}