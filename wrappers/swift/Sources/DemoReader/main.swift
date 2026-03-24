// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

import ZXingCpp
import Foundation

#if canImport(CoreGraphics)
import CoreGraphics
import ImageIO
#endif

func main() throws {
	let args = CommandLine.arguments
	guard args.count >= 2 else {
		let name = (args.first as NSString?)?.lastPathComponent ?? "demo_reader"
		fputs("Usage: \(name) <image_file> [formats] [--fast]\n", stderr)
		fputs("Example: \(name) barcode.png QRCode,EAN13 --fast\n", stderr)
		fputs("Supported formats: \(BarcodeFormats.list(.allReadable).map { $0.description }.joined(separator: ", "))\n", stderr)
		exit(1)
	}

	let filename = args[1]
	let formatsStr = args.count > 2 && !args[2].hasPrefix("--") ? args[2] : ""
	let fast = args.contains("--fast")

	guard let formats = BarcodeFormats(string: formatsStr) else {
		fputs("Error: Invalid format string '\(formatsStr)'\n", stderr)
		fputs("Supported formats: \(BarcodeFormats.list(.allReadable).map { $0.description }.joined(separator: ", "))\n", stderr)
		exit(1)
	}

	let reader = BarcodeReader(formats: formats, tryHarder: !fast, tryRotate: !fast, tryInvert: !fast, tryDownscale: !fast,
		returnErrors: true )

#if canImport(CoreGraphics)
	print("Reading barcodes from '\(filename)'...")

	let url = URL(fileURLWithPath: filename)
	guard let source = CGImageSourceCreateWithURL(url as CFURL, nil) else {
		fputs("Error: Could not create image source for '\(filename)'\n", stderr)
		exit(1)
	}
	guard let cgImage = CGImageSourceCreateImageAtIndex(source, 0, nil) else {
		fputs("Error: Could not load image from '\(filename)'\n", stderr)
		exit(1)
	}

	let barcodes = try reader.read(from: cgImage)

	if barcodes.isEmpty {
		print("No barcode found.")
	} else {
		for barcode in barcodes {
			print("Text:       \(barcode.text)")
			print("Bytes:      \(barcode.bytes.map { String(format: "%02X ", $0) }.joined())")
			print("Format:     \(barcode.format)")
			print("Content:    \(barcode.contentType)")
			print("Identifier: \(barcode.symbologyIdentifier)")
			print("Error:      \(barcode.errorMessage)")
			print("Rotation:   \(barcode.orientation)")
			print("Position:   \(barcode.position)")
			print("Extra:      \(barcode.extra())")
			if barcode != barcodes.last {
				print()
			}
		}
	}
#else
	fputs("Error: CoreGraphics is not available on this platform, cannot read image.\n", stderr)
#endif
}

do {
	try main()
} catch {
	fputs("Error: \(error)\n", stderr)
	exit(1)
}