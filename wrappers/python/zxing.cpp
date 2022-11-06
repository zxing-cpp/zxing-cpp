/*
 * Copyright 2019 Tim Rae
 * Copyright 2021 Antoine Humbert
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"

// Reader
#include "ReadBarcode.h"
#include "ZXAlgorithms.h"

// Writer
#include "BitMatrix.h"
#include "MultiFormatWriter.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <optional>
#include <memory>
#include <vector>

using namespace ZXing;
namespace py = pybind11;

// Numpy array wrapper class for images (either BGR or GRAYSCALE)
using Image = py::array_t<uint8_t, py::array::c_style>;

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	os.seekp(-1, os.cur);
	os << '\0';
	return os;
}

auto read_barcodes_impl(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, TextMode text_mode,
						Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, uint8_t max_number_of_symbols = 0xff)
{
	const auto hints = DecodeHints()
		.setFormats(formats)
		.setTryRotate(try_rotate)
		.setTryDownscale(try_downscale)
		.setTextMode(text_mode)
		.setBinarizer(binarizer)
		.setIsPure(is_pure)
		.setMaxNumberOfSymbols(max_number_of_symbols)
		.setEanAddOnSymbol(ean_add_on_symbol);
	const auto _type = std::string(py::str(py::type::of(_image)));
	Image image;
	try {
		image = _image.cast<Image>();
	}
	catch(...) {
		throw py::type_error("Unsupported type " + _type + ". Expect a PIL Image or numpy array");
	}
	const auto height = narrow_cast<int>(image.shape(0));
	const auto width = narrow_cast<int>(image.shape(1));
	auto channels = image.ndim() == 2 ? 1 : narrow_cast<int>(image.shape(2));
	ImageFormat imgfmt;
	if (_type.find("PIL.") != std::string::npos) {
		const auto mode = _image.attr("mode").cast<std::string>();
		if (mode == "L")
			imgfmt = ImageFormat::Lum;
		else if (mode == "RGB")
			imgfmt = ImageFormat::RGB;
		else if (mode == "RGBA")
			imgfmt = ImageFormat::RGBX;
		else {
			// Unsupported mode in ImageFormat. Let's do conversion to L mode with PIL
			image = _image.attr("convert")("L").cast<Image>();
			imgfmt = ImageFormat::Lum;
			channels = 1;
		}
	} else {
		// Assume grayscale or BGR image depending on channels number
		if (channels == 1)
			imgfmt = ImageFormat::Lum;
		else if (channels == 3)
			imgfmt = ImageFormat::BGR;
		else
			throw py::type_error("Unsupported number of channels for numpy array: " + std::to_string(channels));
	}

	const auto bytes = image.data();
	return ReadBarcodes({bytes, width, height, imgfmt, width * channels, channels}, hints);
}

std::optional<Result> read_barcode(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale,
								   TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol)
{
	auto res = read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol, 1);
	return res.empty() ? std::nullopt : std::optional(res.front());
}

Results read_barcodes(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale,
					  TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol)
{
	return read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol);
}

Image write_barcode(BarcodeFormat format, std::string text, int width, int height, int quiet_zone, int ec_level)
{
	auto writer = MultiFormatWriter(format).setEncoding(CharacterSet::UTF8).setMargin(quiet_zone).setEccLevel(ec_level);
	auto bitmap = writer.encode(text, width, height);

	auto result = Image({bitmap.height(), bitmap.width()});
	auto r = result.mutable_unchecked<2>();
	for (py::ssize_t y = 0; y < r.shape(0); y++)
		for (py::ssize_t x = 0; x < r.shape(1); x++)
			r(y, x) = bitmap.get(narrow_cast<int>(x), narrow_cast<int>(y)) ? 0 : 255;
	return result;
}


PYBIND11_MODULE(zxingcpp, m)
{
	m.doc() = "python bindings for zxing-cpp";

	// forward declaration of BarcodeFormats to fix BarcodeFormat function header typings
	// see https://github.com/zxing-cpp/zxing-cpp/pull/271
	py::class_<BarcodeFormats> pyBarcodeFormats(m, "BarcodeFormats");

	py::enum_<BarcodeFormat>(m, "BarcodeFormat", py::arithmetic{}, "Enumeration of zxing supported barcode formats")
		.value("Aztec", BarcodeFormat::Aztec)
		.value("Codabar", BarcodeFormat::Codabar)
		.value("Code39", BarcodeFormat::Code39)
		.value("Code93", BarcodeFormat::Code93)
		.value("Code128", BarcodeFormat::Code128)
		.value("DataMatrix", BarcodeFormat::DataMatrix)
		.value("EAN8", BarcodeFormat::EAN8)
		.value("EAN13", BarcodeFormat::EAN13)
		.value("ITF", BarcodeFormat::ITF)
		.value("MaxiCode", BarcodeFormat::MaxiCode)
		.value("PDF417", BarcodeFormat::PDF417)
		.value("QRCode", BarcodeFormat::QRCode)
		.value("MircoQRCode", BarcodeFormat::MicroQRCode)
		.value("DataBar", BarcodeFormat::DataBar)
		.value("DataBarExpanded", BarcodeFormat::DataBarExpanded)
		.value("UPCA", BarcodeFormat::UPCA)
		.value("UPCE", BarcodeFormat::UPCE)
		// use upper case 'NONE' because 'None' is a reserved identifier in python
		.value("NONE", BarcodeFormat::None)
		.value("LinearCodes", BarcodeFormat::LinearCodes)
		.value("MatrixCodes", BarcodeFormat::MatrixCodes)
		.export_values()
		// see https://github.com/pybind/pybind11/issues/2221
		.def("__or__", [](BarcodeFormat f1, BarcodeFormat f2){ return f1 | f2; });
	pyBarcodeFormats
		.def("__repr__", py::overload_cast<BarcodeFormats>(static_cast<std::string(*)(BarcodeFormats)>(ToString)))
		.def("__str__", py::overload_cast<BarcodeFormats>(static_cast<std::string(*)(BarcodeFormats)>(ToString)))
		.def("__eq__", [](BarcodeFormats f1, BarcodeFormats f2){ return f1 == f2; })
		.def("__or__", [](BarcodeFormats fs, BarcodeFormat f){ return fs | f; })
		.def(py::init<BarcodeFormat>());
	py::implicitly_convertible<BarcodeFormat, BarcodeFormats>();
	py::enum_<Binarizer>(m, "Binarizer", "Enumeration of binarizers used before decoding images")
		.value("BoolCast", Binarizer::BoolCast)
		.value("FixedThreshold", Binarizer::FixedThreshold)
		.value("GlobalHistogram", Binarizer::GlobalHistogram)
		.value("LocalAverage", Binarizer::LocalAverage)
		.export_values();
	py::enum_<EanAddOnSymbol>(m, "EanAddOnSymbol", "Enumeration of options for EAN-2/5 add-on symbols check")
		.value("Ignore", EanAddOnSymbol::Ignore, "Ignore any Add-On symbol during read/scan")
		.value("Read", EanAddOnSymbol::Read, "Read EAN-2/EAN-5 Add-On symbol if found")
		.value("Require", EanAddOnSymbol::Require, "Require EAN-2/EAN-5 Add-On symbol to be present")
		.export_values();
	py::enum_<ContentType>(m, "ContentType", "Enumeration of content types")
		.value("Text", ContentType::Text)
		.value("Binary", ContentType::Binary)
		.value("Mixed", ContentType::Mixed)
		.value("GS1", ContentType::GS1)
		.value("ISO15434", ContentType::ISO15434)
		.value("UnknownECI", ContentType::UnknownECI)
		.export_values();
	py::enum_<TextMode>(m, "TextMode", "")
		.value("Plain", TextMode::Plain, "bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)")
		.value("ECI", TextMode::ECI, "standard content following the ECI protocol with every character set ECI segment transcoded to unicode")
		.value("HRI", TextMode::HRI, "Human Readable Interpretation (dependent on the ContentType)")
		.value("Hex", TextMode::Hex, "bytes() transcoded to ASCII string of HEX values")
		.value("Escaped", TextMode::Escaped, "Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to '<GS>'")
		.export_values();
	py::class_<PointI>(m, "Point", "Represents the coordinates of a point in an image")
		.def_readonly("x", &PointI::x,
			":return: horizontal coordinate of the point\n"
			":rtype: int")
		.def_readonly("y", &PointI::y,
			":return: vertical coordinate of the point\n"
			":rtype: int");
	py::class_<Position>(m, "Position", "The position of a decoded symbol")
		.def_property_readonly("top_left", &Position::topLeft,
			":return: coordinate of the symbol's top-left corner\n"
			":rtype: zxing.Point")
		.def_property_readonly("top_right", &Position::topRight,
			":return: coordinate of the symbol's top-right corner\n"
			":rtype: zxing.Point")
		.def_property_readonly("bottom_left", &Position::bottomLeft,
			":return: coordinate of the symbol's bottom-left corner\n"
			":rtype: zxing.Point")
		.def_property_readonly("bottom_right", &Position::bottomRight,
			":return: coordinate of the symbol's bottom-right corner\n"
			":rtype: zxing.Point")
		.def("__str__", [](Position pos) {
			std::ostringstream oss;
			oss << pos;
			return oss.str();
		});
	py::class_<Result>(m, "Result", "Result of barcode reading")
		.def_property_readonly("valid", &Result::isValid,
			":return: whether or not result is valid (i.e. a symbol was found)\n"
			":rtype: bool")
		.def_property_readonly("text", [](const Result& res) { return res.text(); },
			":return: text of the decoded symbol (see also TextMode parameter)\n"
			":rtype: str")
		.def_property_readonly("bytes", [](const Result& res) { return py::bytes(res.bytes().asString()); },
			":return: uninterpreted bytes of the decoded symbol\n"
			":rtype: bytes")
		.def_property_readonly("format", &Result::format,
			":return: decoded symbol format\n"
			":rtype: zxing.BarcodeFormat")
		.def_property_readonly("symbology_identifier", &Result::symbologyIdentifier,
			":return: decoded symbology idendifier\n"
			":rtype: str")
		.def_property_readonly("content_type", &Result::contentType,
			":return: content type of symbol\n"
			":rtype: zxing.ContentType")
		.def_property_readonly("position", &Result::position,
			":return: position of the decoded symbol\n"
			":rtype: zxing.Position")
		.def_property_readonly("orientation", &Result::orientation,
			":return: orientation (in degree) of the decoded symbol\n"
			":rtype: int");
	m.def("barcode_format_from_str", &BarcodeFormatFromString,
		py::arg("str"),
		"Convert string to BarcodeFormat\n\n"
		":type str: str\n"
		":param str: string representing barcode format\n"
		":return: corresponding barcode format\n"
		":rtype: zxing.BarcodeFormat");
	m.def("barcode_formats_from_str", &BarcodeFormatsFromString,
		py::arg("str"),
		"Convert string to BarcodeFormats\n\n"
		":type str: str\n"
		":param str: string representing a list of barcodes formats\n"
		":return: corresponding barcode formats\n"
		":rtype: zxing.BarcodeFormats");
	m.def("read_barcode", &read_barcode,
		py::arg("image"),
		py::arg("formats") = BarcodeFormats{},
		py::arg("try_rotate") = true,
		py::arg("try_downscale") = true,
		py::arg("text_mode") = TextMode::HRI,
		py::arg("binarizer") = Binarizer::LocalAverage,
		py::arg("is_pure") = false,
		py::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		"Read (decode) a barcode from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n"
		"  - a PIL Image\n"
		":type formats: zxing.BarcodeFormat|zxing.BarcodeFormats\n"
		":param formats: the format(s) to decode. If ``None``, decode all formats.\n"
		":type try_rotate: bool\n"
		":param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; \n"
		"  if ``False``, it will not search for 90° / 270° rotated barcodes.\n"
		":type try_downscale: bool\n"
		":param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; \n"
		"  if ``False``, it will only search in the resolution provided.\n"
		":type text_mode: zxing.TextMode\n"
		":param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text in the Result.\n"
		"  Defaults to :py:attr:`zxing.TextMode.HRI`."
		":type binarizer: zxing.Binarizer\n"
		":param binarizer: the binarizer used to convert image before decoding barcodes.\n"
		"  Defaults to :py:attr:`zxing.Binarizer.LocalAverage`."
		":type is_pure: bool\n"
		":param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).\n"
		"  Speeds up detection in that case. Default is False."
		":type ean_add_on_symbol: zxing.EanAddOnSymbol\n"
		":param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning \n"
		"  EAN/UPC codes. Default is ``Ignore``.\n"
		":rtype: zxing.Result\n"
		":return: a zxing result containing decoded symbol if found, None otherwise"
	);
	m.def("read_barcodes", &read_barcodes,
		py::arg("image"),
		py::arg("formats") = BarcodeFormats{},
		py::arg("try_rotate") = true,
		py::arg("try_downscale") = true,
		py::arg("text_mode") = TextMode::HRI,
		py::arg("binarizer") = Binarizer::LocalAverage,
		py::arg("is_pure") = false,
		py::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		"Read (decode) multiple barcodes from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n"
		"  - a PIL Image\n"
		":type formats: zxing.BarcodeFormat|zxing.BarcodeFormats\n"
		":param formats: the format(s) to decode. If ``None``, decode all formats.\n"
		":type try_rotate: bool\n"
		":param try_rotate: if ``True`` (the default), decoder searches for barcodes in any direction; \n"
		"  if ``False``, it will not search for 90° / 270° rotated barcodes.\n"
		":type try_downscale: bool\n"
		":param try_downscale: if ``True`` (the default), decoder also scans downscaled versions of the input; \n"
		"  if ``False``, it will only search in the resolution provided.\n"
		":type text_mode: zxing.TextMode\n"
		":param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text in the Result.\n"
		"  Defaults to :py:attr:`zxing.TextMode.HRI`."
		":type binarizer: zxing.Binarizer\n"
		":param binarizer: the binarizer used to convert image before decoding barcodes.\n"
		"  Defaults to :py:attr:`zxing.Binarizer.LocalAverage`."
		":type is_pure: bool\n"
		":param is_pure: Set to True if the input contains nothing but a perfectly aligned barcode (generated image).\n"
		"  Speeds up detection in that case. Default is False."
		":type ean_add_on_symbol: zxing.EanAddOnSymbol\n"
		":param ean_add_on_symbol: Specify whether to Ignore, Read or Require EAN-2/5 add-on symbols while scanning \n"
		"  EAN/UPC codes. Default is ``Ignore``.\n"
		":rtype: zxing.Result\n"
		":return: a list of zxing results containing decoded symbols, the list is empty if none is found"
		);
	m.def("write_barcode", &write_barcode,
		py::arg("format"),
		py::arg("text"),
		py::arg("width") = 0,
		py::arg("height") = 0,
		py::arg("quiet_zone") = -1,
		py::arg("ec_level") = -1,
		"Write (encode) a text into a barcode and return numpy (grayscale) image array\n\n"
		":type format: zxing.BarcodeFormat\n"
		":param format: format of the barcode to create\n"
		":type text: str\n"
		":param text: the text of barcode\n"
		":type width: int\n"
		":param width: width (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be\n"
		"  created with the minimum possible width\n"
		":type height: int\n"
		":param height: height (in pixels) of the barcode to create. If undefined (or set to 0), barcode will be\n"
		"  created with the minimum possible height\n"
		":type quiet_zone: int\n"
		":param quiet_zone: minimum size (in pixels) of the quiet zone around barcode. If undefined (or set to -1), \n"
		"  the minimum quiet zone of respective barcode is used."
		":type ec_level: int\n"
		":param ec_level: error correction level of the barcode\n"
		"  (Used for Aztec, PDF417, and QRCode only)."
	);
}
