// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

import Foundation

// MARK: - CoreGraphics

#if canImport(CoreGraphics)
import CoreGraphics

extension Point {
	/// Converts the point to a `CGPoint` for use with CoreGraphics.
	public var cgPoint: CGPoint {
		CGPoint(x: x, y: y)
	}
}

extension Position {
	/// Returns the corner points as an array of `CGPoint` for drawing operations.
	public var cgPoints: [CGPoint] {
		[topLeft.cgPoint, topRight.cgPoint, bottomRight.cgPoint, bottomLeft.cgPoint]
	}
}

private func grayscaleData(from image: CGImage) throws -> Data {
	var buffer = Data(count: image.width * image.height)

	try buffer.withUnsafeMutableBytes { rawBuffer in
		guard let baseAddress = rawBuffer.baseAddress else {
			throw ZXingError("Could not allocate grayscale conversion buffer")
		}

		guard let context = CGContext(
			data: baseAddress,
			width: image.width,
			height: image.height,
			bitsPerComponent: 8,
			bytesPerRow: image.width,
			space: CGColorSpaceCreateDeviceGray(),
			bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.none.rawValue).rawValue
		) else {
			throw ZXingError("Could not create CGContext for grayscale conversion")
		}

		context.draw(image, in: CGRect(x: 0, y: 0, width: image.width, height: image.height))
	}

	return buffer
}

extension ImageView {
	/// Creates an ImageView from a CGImage. The CGImage pixel data is retained (not copied).
	public convenience init(cgImage: CGImage) throws {
		guard let dataProvider = cgImage.dataProvider, let cfData = dataProvider.data else {
			throw ZXingError("Could not get pixel data from CGImage")
		}

		var format: ImageFormat = .none
		let bitsPerPixel = cgImage.bitsPerPixel
		let bitsPerComponent = cgImage.bitsPerComponent
		let byteOrder = CGBitmapInfo(rawValue: cgImage.bitmapInfo.rawValue & CGBitmapInfo.byteOrderMask.rawValue)

		if bitsPerPixel == 8 && bitsPerComponent == 8 {
			format = .lum
		} else if bitsPerPixel == 32 && bitsPerComponent == 8 {
			switch (cgImage.alphaInfo) {
			case (.premultipliedFirst),
				 (.first),
				 (.noneSkipFirst):
				format = byteOrder == .byteOrder32Big ? .argb : .bgra
			case (.premultipliedLast),
				 (.last),
				 (.noneSkipLast):
				format = byteOrder == .byteOrder32Big ? .rgba : .abgr

			default:
				break
			}
		}

		if format == .none {
			try self.init(data: grayscaleData(from: cgImage), width: cgImage.width, height: cgImage.height, format: .lum)
			return
		}

		guard let ptr = CFDataGetBytePtr(cfData) else {
			throw ZXingError("Could not get byte pointer from CGImage data")
		}

		try self.init(
			pointer: ptr, size: CFDataGetLength(cfData),
			width: cgImage.width, height: cgImage.height,
			format: format, rowStride: cgImage.bytesPerRow,
			retaining: cfData
		)
	}
}

extension Image {
	/// Converts the image to a CGImage (grayscale).
	public var cgImage: CGImage? {
		guard width > 0, height > 0 else { return nil }

		guard let provider = CGDataProvider(data: data as CFData) else { return nil }

		return CGImage(
			width: width, height: height,
			bitsPerComponent: 8, bitsPerPixel: 8, bytesPerRow: width,
			space: CGColorSpaceCreateDeviceGray(),
			bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.none.rawValue),
			provider: provider,
			decode: nil, shouldInterpolate: false,
			intent: .defaultIntent
		)
	}
}

extension Barcode {
	/// Renders the barcode as a CGImage.
	public func toCGImage(_ options: WriterOptions? = nil) throws -> CGImage {
		let image = try toImage(options)
		guard let cg = image.cgImage else {
			throw ZXingError("Failed to create CGImage from barcode image")
		}
		return cg
	}
}

extension BarcodeReader {
	/// Reads barcodes directly from a CGImage.
	public func read(from cgImage: CGImage) throws -> [Barcode] {
		return try read(from: ImageView(cgImage: cgImage))
	}
}
#endif

// MARK: - CoreVideo

#if canImport(CoreVideo)
import CoreVideo

extension ImageView {
	/// Creates an ImageView from a CVPixelBuffer. The pixel buffer must be locked for base-address access.
	internal convenience init(lockedPixelBuffer pixelBuffer: CVPixelBuffer) throws {
		var width = CVPixelBufferGetWidth(pixelBuffer)
		var height = CVPixelBufferGetHeight(pixelBuffer)
		var rowStride = CVPixelBufferGetBytesPerRow(pixelBuffer)
		var pointer = CVPixelBufferGetBaseAddress(pixelBuffer).map { UnsafePointer($0.assumingMemoryBound(to: UInt8.self)) }
		let format: ImageFormat

		switch CVPixelBufferGetPixelFormatType(pixelBuffer) {
		case kCVPixelFormatType_OneComponent8:
			format = .lum

		case kCVPixelFormatType_32BGRA:
			format = .bgra

		case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange,
				kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange,
				kCVPixelFormatType_420YpCbCr8Planar,
				kCVPixelFormatType_420YpCbCr8PlanarFullRange:
			pointer = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0).map { UnsafePointer($0.assumingMemoryBound(to: UInt8.self)) }
			width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0)
			height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0)
			rowStride = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0)
			format = .lum

		default:
			throw ZXingError("Unsupported CVPixelBuffer pixel format (\(CVPixelBufferGetPixelFormatType(pixelBuffer)))")
		}

		guard let pointer else { throw ZXingError("Could not get base address from CVPixelBuffer") }

		try self.init(pointer: pointer, size: rowStride * height, width: width, height: height,
					  format: format, rowStride: rowStride, retaining: pixelBuffer)
	}
}

extension BarcodeReader {
	/// Reads barcodes directly from a CVPixelBuffer (including camera YUV formats).
	public func read(from pixelBuffer: CVPixelBuffer) throws -> [Barcode] {
		CVPixelBufferLockBaseAddress(pixelBuffer, .readOnly)
		defer { CVPixelBufferUnlockBaseAddress(pixelBuffer, .readOnly) }
		return try read(from: ImageView(lockedPixelBuffer: pixelBuffer))
	}
}
#endif

// MARK: - UIKit

#if canImport(UIKit)
import UIKit

extension Barcode {
	/// Renders the barcode as a UIImage.
	public func toUIImage(_ options: WriterOptions? = nil) throws -> UIImage {
		let cgImage = try toCGImage(options)
		return UIImage(cgImage: cgImage)
	}
}

extension BarcodeReader {
	/// Reads barcodes directly from a UIImage.
	public func read(from image: UIImage) throws -> [Barcode] {
		guard let cgImage = image.cgImage else {
			throw ZXingError("Could not get CGImage from UIImage")
		}
		return try read(from: cgImage)
	}
}
#endif

// MARK: - AppKit

#if canImport(AppKit) && !targetEnvironment(macCatalyst)
import AppKit

extension Barcode {
	/// Renders the barcode as an NSImage.
	public func toNSImage(_ options: WriterOptions? = nil) throws -> NSImage {
		let cgImage = try toCGImage(options)
		return NSImage(cgImage: cgImage, size: NSSize(width: cgImage.width, height: cgImage.height))
	}
}

extension BarcodeReader {
	/// Reads barcodes directly from an NSImage.
	public func read(from image: NSImage) throws -> [Barcode] {
		guard let cgImage = image.cgImage(forProposedRect: nil, context: nil, hints: nil) else {
			throw ZXingError("Could not get CGImage from NSImage")
		}
		return try read(from: cgImage)
	}
}
#endif
