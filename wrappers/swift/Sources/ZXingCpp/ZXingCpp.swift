// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

import ZXingCBridge
import Foundation

// MARK: - Error

/// Error type for ZXing operations.
public struct ZXingError: Error, LocalizedError, CustomStringConvertible {
	public let message: String
	public var description: String { message }
	public var errorDescription: String? { message }

	init(_ message: String) {
		self.message = message
	}
}

// MARK: - Internal Helpers

private func c2s(_ ptr: UnsafeMutablePointer<CChar>?) -> String {
	guard let ptr else { return "" }
	let str = String(cString: ptr)
	ZXing_free(ptr)
	return str
}

private func c2bytes(_ ptr: UnsafeMutablePointer<UInt8>?, _ len: Int32) -> Data {
	guard let ptr, len > 0 else { return Data() }
	let data = Data(bytes: ptr, count: Int(len))
	ZXing_free(ptr)
	return data
}

private func lastError() -> ZXingError {
	if let msg = ZXing_LastErrorMsg() {
		return ZXingError(c2s(msg))
	}
	return ZXingError("Unknown ZXing error")
}

/// Bridge our Int32-based Swift types to/from C enum types (imported with UInt32 rawValue).
private func cEnum<T: RawRepresentable>(_ v: Int32) -> T where T.RawValue == UInt32 {
	T(rawValue: UInt32(bitPattern: v))!
}
private func sEnum<T: RawRepresentable>(_ v: T) -> Int32 where T.RawValue == UInt32 {
	Int32(bitPattern: v.rawValue)
}

/// Returns the native zxing-cpp library version string.
public func version() -> String {
	guard let v = ZXing_Version() else { return "" }
	return String(cString: v)
}

// MARK: - BarcodeFormat

/// Represents a barcode format/symbology.
///
/// Can represent a specific format (e.g., `.qrCode`) or a filter for multiple formats (e.g., `.allReadable`).
public struct BarcodeFormat: RawRepresentable, Hashable, Sendable, CustomStringConvertible {
	public let rawValue: Int32
	public init(rawValue: Int32) { self.rawValue = rawValue }

	public var description: String { c2s(ZXing_BarcodeFormatToString(cEnum(rawValue))) }

	/// The base symbology for this format (e.g., `.ean13` returns `.eanUPC`).
	public var symbology: BarcodeFormat { BarcodeFormat(rawValue: sEnum(ZXing_BarcodeFormatSymbology(cEnum(rawValue)))) }

	/// Parses a format name string into a `BarcodeFormat`. Returns `nil` on failure.
	public init?(string: String) {
		let value = ZXing_BarcodeFormatFromString(string)
		if sEnum(value) == BarcodeFormat.invalid.rawValue { return nil }
		self.rawValue = sEnum(value)
	}


	// Filter constants
	public static let invalid           = BarcodeFormat(rawValue: 0xFFFF)
	public static let none              = BarcodeFormat(rawValue: 0x0000)
	public static let all               = BarcodeFormat(rawValue: 0x2A2A)
	public static let allReadable       = BarcodeFormat(rawValue: 0x722A)
	public static let allCreatable      = BarcodeFormat(rawValue: 0x772A)
	public static let allLinear         = BarcodeFormat(rawValue: 0x6C2A)
	public static let allMatrix         = BarcodeFormat(rawValue: 0x6D2A)
	public static let allGS1            = BarcodeFormat(rawValue: 0x672A)

	// DataBar
	public static let dataBar           = BarcodeFormat(rawValue: 0x2065)
	public static let dataBarOmni       = BarcodeFormat(rawValue: 0x6F65)
	public static let dataBarStk        = BarcodeFormat(rawValue: 0x7365)
	public static let dataBarStkOmni    = BarcodeFormat(rawValue: 0x4F65)
	public static let dataBarLtd        = BarcodeFormat(rawValue: 0x6C65)
	public static let dataBarExp        = BarcodeFormat(rawValue: 0x6565)
	public static let dataBarExpStk     = BarcodeFormat(rawValue: 0x4565)

	// EAN/UPC
	public static let eanUPC            = BarcodeFormat(rawValue: 0x2045)
	public static let ean13             = BarcodeFormat(rawValue: 0x3145)
	public static let ean8              = BarcodeFormat(rawValue: 0x3845)
	public static let ean5              = BarcodeFormat(rawValue: 0x3545)
	public static let ean2              = BarcodeFormat(rawValue: 0x3245)
	public static let isbn              = BarcodeFormat(rawValue: 0x6945)
	public static let upcA              = BarcodeFormat(rawValue: 0x6145)
	public static let upcE              = BarcodeFormat(rawValue: 0x6545)

	// Code39
	public static let code39            = BarcodeFormat(rawValue: 0x2041)
	public static let code39Std         = BarcodeFormat(rawValue: 0x7341)
	public static let code39Ext         = BarcodeFormat(rawValue: 0x6541)
	public static let code32            = BarcodeFormat(rawValue: 0x3241)
	public static let pzn               = BarcodeFormat(rawValue: 0x7041)

	// Other linear
	public static let codabar           = BarcodeFormat(rawValue: 0x2046)
	public static let code93            = BarcodeFormat(rawValue: 0x2047)
	public static let code128           = BarcodeFormat(rawValue: 0x2043)
	public static let itf               = BarcodeFormat(rawValue: 0x2049)
	public static let itf14             = BarcodeFormat(rawValue: 0x3449)
	public static let otherBarcode      = BarcodeFormat(rawValue: 0x2058)
	public static let dxFilmEdge        = BarcodeFormat(rawValue: 0x7858)

	// PDF417
	public static let pdf417            = BarcodeFormat(rawValue: 0x204C)
	public static let compactPDF417     = BarcodeFormat(rawValue: 0x634C)
	public static let microPDF417       = BarcodeFormat(rawValue: 0x6D4C)

	// Aztec
	public static let aztec             = BarcodeFormat(rawValue: 0x207A)
	public static let aztecCode         = BarcodeFormat(rawValue: 0x637A)
	public static let aztecRune         = BarcodeFormat(rawValue: 0x727A)

	// QR Code
	public static let qrCode            = BarcodeFormat(rawValue: 0x2051)
	public static let qrCodeModel1      = BarcodeFormat(rawValue: 0x3151)
	public static let qrCodeModel2      = BarcodeFormat(rawValue: 0x3251)
	public static let microQRCode       = BarcodeFormat(rawValue: 0x6D51)
	public static let rmqrCode          = BarcodeFormat(rawValue: 0x7251)

	// Other matrix
	public static let dataMatrix        = BarcodeFormat(rawValue: 0x2064)
	public static let maxiCode          = BarcodeFormat(rawValue: 0x2055)
}

public typealias BarcodeFormats = [BarcodeFormat]

extension Array where Element == BarcodeFormat {
	/// Returns individual formats matching the given filter.
	public static func list(_ filter: BarcodeFormat = .all) -> Self {
		var count: Int32 = 0
		guard let ptr = ZXing_BarcodeFormatsList(cEnum(filter.rawValue), &count), count > 0 else { return [] }
		let formats = (0..<Int(count)).map { BarcodeFormat(rawValue: sEnum(ptr[$0])) }
		ZXing_free(ptr)
		return formats
	}

	/// Parses a comma-separated string of format names into an array of `BarcodeFormat`. Returns `nil` on failure.
	public init?(string: String) {
		var count: Int32 = 0
		let ptr = string.withCString { ZXing_BarcodeFormatsFromString($0, &count) }
		if ptr != nil && count > 0 {
			self = (0..<Int(count)).map { BarcodeFormat(rawValue: sEnum(ptr![$0])) }
			ZXing_free(ptr)
		} else if let msg = ZXing_LastErrorMsg() {
			ZXing_free(msg)
			return nil
		} else {
			self = []
		}
	}
}

private extension Array where Element == BarcodeFormat {
	func withCFormats<T>(_ body: (UnsafePointer<ZXing_BarcodeFormat>?, Int32) -> T) -> T {
		let raw: [ZXing_BarcodeFormat] = map { cEnum($0.rawValue) }
		return raw.withUnsafeBufferPointer { buf in
			body(buf.baseAddress, Int32(buf.count))
		}
	}
}

// MARK: - Enums

public enum ImageFormat: Int32, Sendable {
	case none = 0
	case lum  = 0x01000000
	case lumA = 0x02000000
	case rgb  = 0x03000102
	case bgr  = 0x03020100
	case rgba = 0x04000102
	case argb = 0x04010203
	case bgra = 0x04020100
	case abgr = 0x04030201
}

public enum ContentType: Int32, Sendable, CustomStringConvertible {
	case text       = 0
	case binary     = 1
	case mixed      = 2
	case gs1        = 3
	case iso15434   = 4
	case unknownECI = 5

	public var description: String { c2s(ZXing_ContentTypeToString(cEnum(rawValue))) }
}

public enum ErrorType: Int32, Sendable {
	case none        = 0
	case format      = 1
	case checksum    = 2
	case unsupported = 3
}

public enum Binarizer: Int32, Sendable {
	case localAverage    = 0
	case globalHistogram = 1
	case fixedThreshold  = 2
	case boolCast        = 3
}

public enum EanAddOnSymbol: Int32, Sendable {
	case ignore  = 0
	case read    = 1
	case require = 2
}

public enum TextMode: Int32, Sendable {
	case plain   = 0
	case eci     = 1
	case hri     = 2
	case escaped = 3
	case hex     = 4
	case hexECI  = 5
}

// MARK: - Point & Position

public struct Point: Hashable, Sendable, CustomStringConvertible {
	public var x: Int
	public var y: Int

	public var description: String { "\(x)x\(y)" }

	public init(x: Int, y: Int) { self.x = x; self.y = y }

	init(_ p: ZXing_PointI) { x = Int(p.x); y = Int(p.y) }
}

public struct Position: Sendable, CustomStringConvertible {
	public var topLeft: Point
	public var topRight: Point
	public var bottomRight: Point
	public var bottomLeft: Point

	public var description: String {
		c2s(ZXing_PositionToString(cPosition))
	}

	init(_ p: ZXing_Position) {
		topLeft = Point(p.topLeft)
		topRight = Point(p.topRight)
		bottomRight = Point(p.bottomRight)
		bottomLeft = Point(p.bottomLeft)
	}

	private var cPosition: ZXing_Position {
		ZXing_Position(
			topLeft: ZXing_PointI(x: Int32(topLeft.x), y: Int32(topLeft.y)),
			topRight: ZXing_PointI(x: Int32(topRight.x), y: Int32(topRight.y)),
			bottomRight: ZXing_PointI(x: Int32(bottomRight.x), y: Int32(bottomRight.y)),
			bottomLeft: ZXing_PointI(x: Int32(bottomLeft.x), y: Int32(bottomLeft.y))
		)
	}
}

// MARK: - ImageView

/// A non-owning view into image pixel data for barcode detection.
public class ImageView {
	internal let _handle: OpaquePointer
	private let _retainedSource: AnyObject?

	private static func makeHandle(
		pointer: UnsafePointer<UInt8>?, size: Int, width: Int, height: Int, format: ImageFormat, rowStride: Int, pixStride: Int
	) throws -> OpaquePointer {
		guard let iv = ZXing_ImageView_new_checked(
			pointer, Int32(size), Int32(width), Int32(height), cEnum(format.rawValue), Int32(rowStride), Int32(pixStride)
		) else {
			throw lastError()
		}
		return iv
	}

	/// Creates an ImageView from Data without additional pixel buffer copying.
	public init( data: Data, width: Int, height: Int, format: ImageFormat, rowStride: Int = 0, pixStride: Int = 0) throws {
		_handle = try data.withUnsafeBytes { rawBuffer in
			return try Self.makeHandle(pointer: rawBuffer.bindMemory(to: UInt8.self).baseAddress, size: data.count,
				width: width, height: height, format: format, rowStride: rowStride, pixStride: pixStride)
		}
		_retainedSource = data as NSData
	}

	/// Creates an ImageView from an external data source. The source is retained to keep the pointer valid.
	public init(
		pointer: UnsafePointer<UInt8>, size: Int, width: Int, height: Int, format: ImageFormat,
		rowStride: Int = 0, pixStride: Int = 0, retaining source: AnyObject
	) throws {
		_handle = try Self.makeHandle(
			pointer: pointer, size: size, width: width, height: height, format: format, rowStride: rowStride, pixStride: pixStride)
		_retainedSource = source
	}

	deinit {
		ZXing_ImageView_delete(_handle)
	}

	/// Crops the image view to the given rectangle.
	public func crop(left: Int, top: Int, width: Int, height: Int) {
		ZXing_ImageView_crop(_handle, Int32(left), Int32(top), Int32(width), Int32(height))
	}

	/// Rotates the image view by the given degrees (must be a multiple of 90).
	public func rotate(by degrees: Int) {
		ZXing_ImageView_rotate(_handle, Int32(degrees))
	}
}

// MARK: - Image

/// An owned image returned by barcode writing operations.
public class Image {
	internal let _handle: OpaquePointer

	internal init(_ handle: OpaquePointer) { _handle = handle }

	deinit { ZXing_Image_delete(_handle) }

	public var width: Int { Int(ZXing_Image_width(_handle)) }
	public var height: Int { Int(ZXing_Image_height(_handle)) }
	public var format: ImageFormat { ImageFormat(rawValue: sEnum(ZXing_Image_format(_handle))) ?? .none }

	/// The raw pixel data as a copy.
	public var data: Data {
		guard let ptr = ZXing_Image_data(_handle) else { return Data() }
		return Data(bytes: ptr, count: width * height)
	}
}

// MARK: - WriterOptions

/// Options for rendering barcodes to images or SVG.
public class WriterOptions {
	internal let _handle: OpaquePointer

	public init() {
		_handle = ZXing_WriterOptions_new()!
	}

	public convenience init( scale: Int? = nil,	rotate: Int? = nil,	addHRT: Bool? = nil, addQuietZones: Bool? = nil) {
		self.init()
		if let scale { self.scale = scale }
		if let rotate { self.rotate = rotate }
		if let addHRT { self.addHRT = addHRT }
		if let addQuietZones { self.addQuietZones = addQuietZones }
	}

	deinit { ZXing_WriterOptions_delete(_handle) }

	/// Scaling factor (>0: pixels per module, <0: target size in pixels).
	public var scale: Int {
		get { Int(ZXing_WriterOptions_getScale(_handle)) }
		set { ZXing_WriterOptions_setScale(_handle, Int32(newValue)) }
	}

	/// Rotation in degrees (0, 90, 180, or 270).
	public var rotate: Int {
		get { Int(ZXing_WriterOptions_getRotate(_handle)) }
		set { ZXing_WriterOptions_setRotate(_handle, Int32(newValue)) }
	}

	/// Add human-readable text below linear barcodes.
	public var addHRT: Bool {
		get { ZXing_WriterOptions_getAddHRT(_handle) }
		set { ZXing_WriterOptions_setAddHRT(_handle, newValue) }
	}

	/// Add quiet zones (white margins) around the barcode.
	public var addQuietZones: Bool {
		get { ZXing_WriterOptions_getAddQuietZones(_handle) }
		set { ZXing_WriterOptions_setAddQuietZones(_handle, newValue) }
	}
}

// MARK: - Barcode

/// A detected or created barcode.
public class Barcode: Equatable, Hashable {
	internal let _handle: OpaquePointer

	internal init(_ handle: OpaquePointer) { _handle = handle }

	deinit { ZXing_Barcode_delete(_handle) }

	/// Creates a barcode from text content.
	public convenience init(_ text: String, format: BarcodeFormat, options: String? = nil) throws {
		guard let opts = ZXing_CreatorOptions_new(cEnum(format.rawValue)) else { throw lastError() }
		defer { ZXing_CreatorOptions_delete(opts) }
		if let options {
			options.withCString { ZXing_CreatorOptions_setOptions(opts, $0) }
		}
		guard let bc = ZXing_CreateBarcodeFromText(text, 0, opts) else { throw lastError() }
		self.init(bc)
	}

	/// Creates a barcode from binary data.
	public convenience init(bytes: Data, format: BarcodeFormat, options: String? = nil) throws {
		guard let opts = ZXing_CreatorOptions_new(cEnum(format.rawValue)) else { throw lastError() }
		defer { ZXing_CreatorOptions_delete(opts) }
		if let options {
			options.withCString { ZXing_CreatorOptions_setOptions(opts, $0) }
		}
		let bc = bytes.withUnsafeBytes { buffer in
			ZXing_CreateBarcodeFromBytes(buffer.baseAddress, Int32(buffer.count), opts)
		}
		guard let bc else { throw lastError() }
		self.init(bc)
	}

	/// Whether the barcode was successfully decoded or created.
	public var isValid: Bool { ZXing_Barcode_isValid(_handle) }

	/// The barcode format (e.g., `.qrCode`, `.ean13`).
	public var format: BarcodeFormat { BarcodeFormat(rawValue: sEnum(ZXing_Barcode_format(_handle))) }

	/// The base symbology (e.g., `.ean13` returns `.eanUPC`).
	public var symbology: BarcodeFormat { BarcodeFormat(rawValue: sEnum(ZXing_Barcode_symbology(_handle))) }

	/// The content type of the decoded data.
	public var contentType: ContentType { ContentType(rawValue: sEnum(ZXing_Barcode_contentType(_handle)))! }

	/// The decoded text content.
	public var text: String { c2s(ZXing_Barcode_text(_handle)) }

	/// The raw decoded bytes.
	public var bytes: Data {
		var len: Int32 = 0
		return c2bytes(ZXing_Barcode_bytes(_handle, &len), len)
	}

	/// The decoded bytes with ECI markers included.
	public var bytesECI: Data {
		var len: Int32 = 0
		return c2bytes(ZXing_Barcode_bytesECI(_handle, &len), len)
	}

	/// ISO/IEC 15424 symbology identifier (e.g., "]Q1" for QR Code).
	public var symbologyIdentifier: String { c2s(ZXing_Barcode_symbologyIdentifier(_handle)) }

	/// Corner points of the barcode in the image.
	public var position: Position { Position(ZXing_Barcode_position(_handle)) }

	/// Detected rotation in degrees.
	public var orientation: Int { Int(ZXing_Barcode_orientation(_handle)) }

	/// Whether the barcode uses Extended Channel Interpretation.
	public var hasECI: Bool { ZXing_Barcode_hasECI(_handle) }

	/// Whether the barcode was detected as light-on-dark.
	public var isInverted: Bool { ZXing_Barcode_isInverted(_handle) }

	/// Whether the barcode was detected as mirrored.
	public var isMirrored: Bool { ZXing_Barcode_isMirrored(_handle) }

	/// Number of detected scan lines (for linear barcodes).
	public var lineCount: Int { Int(ZXing_Barcode_lineCount(_handle)) }

	/// Index of this barcode in a structured append sequence.
	public var sequenceIndex: Int { Int(ZXing_Barcode_sequenceIndex(_handle)) }

	/// Total number of barcodes in a structured append sequence.
	public var sequenceSize: Int { Int(ZXing_Barcode_sequenceSize(_handle)) }

	/// Identifier of the structured append sequence.
	public var sequenceId: String { c2s(ZXing_Barcode_sequenceId(_handle)) }

	/// The error type if decoding encountered an issue.
	public var errorType: ErrorType { ErrorType(rawValue: sEnum(ZXing_Barcode_errorType(_handle)))! }

	/// The error message if decoding encountered an issue.
	public var errorMessage: String { c2s(ZXing_Barcode_errorMsg(_handle)) }

	/// Additional format-specific metadata as JSON.
	/// - Parameter key: Optional key to retrieve a specific value. Pass `nil` for the full JSON object.
	public func extra(key: String? = nil) -> String {
		if let key {
			return key.withCString { c2s(ZXing_Barcode_extra(_handle, $0)) }
		}
		return c2s(ZXing_Barcode_extra(_handle, nil))
	}

	/// Renders the barcode as an SVG string.
	public func toSVG(_ options: WriterOptions? = nil) throws -> String {
		guard let ptr = ZXing_WriteBarcodeToSVG(_handle, options?._handle) else { throw lastError() }
		return c2s(ptr)
	}

	/// Renders the barcode as a grayscale image.
	public func toImage(_ options: WriterOptions? = nil) throws -> Image {
		guard let ptr = ZXing_WriteBarcodeToImage(_handle, options?._handle) else { throw lastError() }
		return Image(ptr)
	}

	public static func == (lhs: Barcode, rhs: Barcode) -> Bool {
		lhs._handle == rhs._handle
	}

	public func hash(into hasher: inout Hasher) {
		hasher.combine(UInt(bitPattern: _handle))
	}
}

// MARK: - BarcodeReader

/// Barcode reader with configurable detection options.
///
/// Usage:
/// ```swift
/// let reader = BarcodeReader(formats: [.qrCode, .ean13], returnErrors: true)
/// let barcodes = try reader.read(from: imageView)
/// ```
public class BarcodeReader {
	internal let _handle: OpaquePointer

	public init() {
		_handle = ZXing_ReaderOptions_new()!
	}

	public convenience init(
		formats: [BarcodeFormat]? = nil,
		tryHarder: Bool? = nil,
		tryRotate: Bool? = nil,
		tryInvert: Bool? = nil,
		tryDownscale: Bool? = nil,
		isPure: Bool? = nil,
		returnErrors: Bool? = nil,
		binarizer: Binarizer? = nil,
		textMode: TextMode? = nil,
		minLineCount: Int? = nil,
		maxNumberOfSymbols: Int? = nil,
		eanAddOnSymbol: EanAddOnSymbol? = nil,
		validateOptionalChecksum: Bool? = nil
	) {
		self.init()
		if let formats { self.formats = formats }
		if let tryHarder { self.tryHarder = tryHarder }
		if let tryRotate { self.tryRotate = tryRotate }
		if let tryInvert { self.tryInvert = tryInvert }
		if let tryDownscale { self.tryDownscale = tryDownscale }
		if let isPure { self.isPure = isPure }
		if let returnErrors { self.returnErrors = returnErrors }
		if let binarizer { self.binarizer = binarizer }
		if let textMode { self.textMode = textMode }
		if let minLineCount { self.minLineCount = minLineCount }
		if let maxNumberOfSymbols { self.maxNumberOfSymbols = maxNumberOfSymbols }
		if let eanAddOnSymbol { self.eanAddOnSymbol = eanAddOnSymbol }
		if let validateOptionalChecksum { self.validateOptionalChecksum = validateOptionalChecksum }
	}

	deinit { ZXing_ReaderOptions_delete(_handle) }

	/// Barcode formats to search for. Empty means all supported formats.
	public var formats: [BarcodeFormat] {
		get {
			var count: Int32 = 0
			guard let ptr = ZXing_ReaderOptions_getFormats(_handle, &count), count > 0 else { return [] }
			let result = (0..<Int(count)).map { BarcodeFormat(rawValue: sEnum(ptr[$0])) }
			ZXing_free(ptr)
			return result
		}
		set {
			newValue.withCFormats { ptr, count in
				ZXing_ReaderOptions_setFormats(_handle, ptr, count)
			}
		}
	}

	/// Spend more time to find barcodes; slower but more accurate.
	public var tryHarder: Bool {
		get { ZXing_ReaderOptions_getTryHarder(_handle) }
		set { ZXing_ReaderOptions_setTryHarder(_handle, newValue) }
	}

	/// Also detect barcodes in 90/180/270 degree rotated images.
	public var tryRotate: Bool {
		get { ZXing_ReaderOptions_getTryRotate(_handle) }
		set { ZXing_ReaderOptions_setTryRotate(_handle, newValue) }
	}

	/// Also try detecting inverted (white on black) barcodes.
	public var tryInvert: Bool {
		get { ZXing_ReaderOptions_getTryInvert(_handle) }
		set { ZXing_ReaderOptions_setTryInvert(_handle, newValue) }
	}

	/// Try downscaled images (high resolution images can hamper the detection).
	public var tryDownscale: Bool {
		get { ZXing_ReaderOptions_getTryDownscale(_handle) }
		set { ZXing_ReaderOptions_setTryDownscale(_handle, newValue) }
	}

	/// Assume the image contains only a single, perfectly aligned barcode.
	public var isPure: Bool {
		get { ZXing_ReaderOptions_getIsPure(_handle) }
		set { ZXing_ReaderOptions_setIsPure(_handle, newValue) }
	}

	/// Return invalid barcodes with error information instead of skipping them.
	public var returnErrors: Bool {
		get { ZXing_ReaderOptions_getReturnErrors(_handle) }
		set { ZXing_ReaderOptions_setReturnErrors(_handle, newValue) }
	}

	/// The binarization algorithm for converting grayscale to black/white.
	public var binarizer: Binarizer {
		get { Binarizer(rawValue: sEnum(ZXing_ReaderOptions_getBinarizer(_handle)))! }
		set { ZXing_ReaderOptions_setBinarizer(_handle, cEnum(newValue.rawValue)) }
	}

	/// Text encoding mode for converting barcode content bytes to strings.
	public var textMode: TextMode {
		get { TextMode(rawValue: sEnum(ZXing_ReaderOptions_getTextMode(_handle)))! }
		set { ZXing_ReaderOptions_setTextMode(_handle, cEnum(newValue.rawValue)) }
	}

	/// Minimum number of lines for linear barcodes (default is 2).
	public var minLineCount: Int {
		get { Int(ZXing_ReaderOptions_getMinLineCount(_handle)) }
		set { ZXing_ReaderOptions_setMinLineCount(_handle, Int32(newValue)) }
	}

	/// Maximum number of symbols to detect.
	public var maxNumberOfSymbols: Int {
		get { Int(ZXing_ReaderOptions_getMaxNumberOfSymbols(_handle)) }
		set { ZXing_ReaderOptions_setMaxNumberOfSymbols(_handle, Int32(newValue)) }
	}

	/// Handling of EAN-2/EAN-5 Add-On symbols.
	public var eanAddOnSymbol: EanAddOnSymbol {
		get { EanAddOnSymbol(rawValue: sEnum(ZXing_ReaderOptions_getEanAddOnSymbol(_handle)))! }
		set { ZXing_ReaderOptions_setEanAddOnSymbol(_handle, cEnum(newValue.rawValue)) }
	}

	/// Validate optional checksums (e.g., Code39, ITF).
	public var validateOptionalChecksum: Bool {
		get { ZXing_ReaderOptions_getValidateOptionalChecksum(_handle) }
		set { ZXing_ReaderOptions_setValidateOptionalChecksum(_handle, newValue) }
	}

	/// Reads barcodes from an image view using this reader's options.
	public func read(from image: ImageView) throws -> [Barcode] {
		guard let barcodes = ZXing_ReadBarcodes(image._handle, _handle) else { return [] }
		defer { ZXing_Barcodes_delete(barcodes) }

		let size = ZXing_Barcodes_size(barcodes)
		guard size > 0 else { return [] }

		return (0..<Int32(size)).map { i in
			Barcode(ZXing_Barcodes_move(barcodes, i)!)
		}
	}
}
