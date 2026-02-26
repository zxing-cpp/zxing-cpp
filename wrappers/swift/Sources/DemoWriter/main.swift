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
	guard args.count >= 4 else {
		let name = (args.first as NSString?)?.lastPathComponent ?? "demo_writer"
		fputs("Usage: \(name) <text> <format> <output.[svg|png]> [options]\n", stderr)
		fputs("Example: \(name) \"Hello World\" QRCode barcode.png \"EcLevel=H\"\n", stderr)
		fputs("Supported formats: \(BarcodeFormats.list(.allCreatable).map { $0.description }.joined(separator: ", "))\n", stderr)
		exit(1)
	}

	let text = args[1]
	let formatStr = args[2]
	let filename = args[3]
	let options = args.count > 4 ? args[4] : nil

	guard let format = BarcodeFormat(string: formatStr) else {
		fputs("Error: Unknown barcode format '\(formatStr)'\n", stderr)
		fputs("Supported formats: \(BarcodeFormats.list(.allCreatable).map { $0.description }.joined(separator: ", "))\n", stderr)
		exit(1)
	}

	print("Creating \(format) barcode for text '\(text)' and writing to '\(filename)'.")

	let barcode = try Barcode(text, format: format, options: options)

	if filename.hasSuffix(".svg") {
		let svg = try barcode.toSVG(WriterOptions(scale: 5, addHRT: true))
		try svg.write(toFile: filename, atomically: true, encoding: .utf8)
	} else {
#if canImport(CoreGraphics)
		let cgImage = try barcode.toCGImage(WriterOptions(scale: 4))

		let url = URL(fileURLWithPath: filename) as CFURL
		guard let dest = CGImageDestinationCreateWithURL(url, "public.png" as CFString, 1, nil) else {
			fputs("Error: Could not create image destination for '\(filename)'\n", stderr)
			exit(1)
		}
		CGImageDestinationAddImage(dest, cgImage, nil)
		guard CGImageDestinationFinalize(dest) else {
			fputs("Error: Could not write image to '\(filename)'\n", stderr)
			exit(1)
		}
#else
		fputs("Error: CoreGraphics is not available on this platform, cannot write image.\n", stderr)
#endif
	}
}

do {
	try main()
} catch {
	fputs("Error: \(error)\n", stderr)
	exit(1)
}
