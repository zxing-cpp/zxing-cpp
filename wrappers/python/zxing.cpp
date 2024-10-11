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
#ifdef ZXING_EXPERIMENTAL_API
#include "WriteBarcode.h"
#else
#include "BitMatrix.h"
#include "Matrix.h"
#include "MultiFormatWriter.h"
#include <cstring>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <optional>
#include <sstream>
#include <vector>

using namespace ZXing;
namespace py = pybind11;
using namespace pybind11::literals; // to bring in the `_a` literal

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	os.seekp(-1, os.cur);
	os << '\0';
	return os;
}

auto read_barcodes_impl(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, TextMode text_mode,
						Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, bool return_errors,
						uint8_t max_number_of_symbols = 0xff)
{
	const auto opts = ReaderOptions()
		.setFormats(formats)
		.setTryRotate(try_rotate)
		.setTryDownscale(try_downscale)
		.setTextMode(text_mode)
		.setBinarizer(binarizer)
		.setIsPure(is_pure)
		.setMaxNumberOfSymbols(max_number_of_symbols)
		.setEanAddOnSymbol(ean_add_on_symbol)
		.setReturnErrors(return_errors);
	const auto _type = std::string(py::str(py::type::of(_image)));
	py::buffer_info info;
	ImageFormat imgfmt = ImageFormat::None;
	try {
		if (py::hasattr(_image, "__array_interface__")) {
			if (_type.find("PIL.") != std::string::npos) {
				_image.attr("load")();
				const auto mode = _image.attr("mode").cast<std::string>();
				if (mode == "L")
					imgfmt = ImageFormat::Lum;
				else if (mode == "RGB")
					imgfmt = ImageFormat::RGB;
				else if (mode == "RGBA")
					imgfmt = ImageFormat::RGBA;
				else {
					// Unsupported mode in ImageFormat. Let's do conversion to L mode with PIL.
					_image = _image.attr("convert")("L");
					imgfmt = ImageFormat::Lum;
				}
			}

			auto ai = _image.attr("__array_interface__").cast<py::dict>();
			auto shape = ai["shape"].cast<std::vector<py::ssize_t>>();
			auto typestr = ai["typestr"].cast<std::string>();

			if (typestr != "|u1")
				throw py::type_error("Incompatible __array_interface__ data type (" + typestr + "): expected a uint8_t array (|u1).");

			if (ai.contains("data")) {
				auto adata = ai["data"];

				if (py::isinstance<py::buffer>(adata)) {
					// PIL and our own __array_interface__ passes data as a buffer/bytes object
					info = adata.cast<py::buffer>().request();
					// PIL's bytes object has wrong dim/shape/strides info
					if (info.ndim != Size(shape)) {
						info.ndim = Size(shape);
						info.shape = shape;
						info.strides = py::detail::c_strides(shape, 1);
					}
				} else if (py::isinstance<py::tuple>(adata)) {
					// numpy data is passed as a tuple
					auto strides = py::detail::c_strides(shape, 1);
					if (ai.contains("strides") && !ai["strides"].is_none())
						strides = ai["strides"].cast<std::vector<py::ssize_t>>();
					auto data_ptr = reinterpret_cast<void*>(adata.cast<py::tuple>()[0].cast<py::size_t>());
					info = py::buffer_info(data_ptr, 1, "B", Size(shape), shape, strides);
				} else {
					throw py::type_error("No way to get data from __array_interface__");
				}
			} else {
				info = _image.cast<py::buffer>().request();
			}
		} else {
			info = _image.cast<py::buffer>().request();
		}
#if PYBIND11_VERSION_HEX > 0x02080000 // py::raise_from is available starting from 2.8.0
	} catch (py::error_already_set &e) {
		py::raise_from(e, PyExc_TypeError, ("Invalid input: " + _type + " does not support the buffer protocol.").c_str());
		throw py::error_already_set();
#endif
	} catch (...) {
		throw py::type_error("Invalid input: " + _type + " does not support the buffer protocol.");
	}

	if (info.format != py::format_descriptor<uint8_t>::format())
		throw py::type_error("Incompatible buffer format '" + info.format + "': expected a uint8_t array.");

	if (info.ndim != 2 && info.ndim != 3)
		throw py::type_error("Incompatible buffer dimension " + std::to_string(info.ndim) + " (needs to be 2 or 3).");

	const auto height = narrow_cast<int>(info.shape[0]);
	const auto width = narrow_cast<int>(info.shape[1]);
	const auto channels = info.ndim == 2 ? 1 : narrow_cast<int>(info.shape[2]);
	const auto rowStride = narrow_cast<int>(info.strides[0]);
	const auto pixStride = narrow_cast<int>(info.strides[1]);
	if (imgfmt == ImageFormat::None) {
		// Assume grayscale or BGR image depending on channels number
		if (channels == 1)
			imgfmt = ImageFormat::Lum;
		else if (channels == 3)
			imgfmt = ImageFormat::BGR;
		else
			throw py::value_error("Unsupported number of channels for buffer: " + std::to_string(channels));
	}

	const auto bytes = static_cast<uint8_t*>(info.ptr);
	// Disables the GIL during zxing processing (restored automatically upon completion)
	py::gil_scoped_release release;
	return ReadBarcodes({bytes, width, height, imgfmt, rowStride, pixStride}, opts);
}

std::optional<Barcode> read_barcode(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale,
									TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol,
									bool return_errors)
{
	auto res = read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol,
								  return_errors, 1);
	return res.empty() ? std::nullopt : std::optional(res.front());
}

Barcodes read_barcodes(py::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, TextMode text_mode,
					   Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, bool return_errors)
{
	return read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol,
							  return_errors);
}

#ifdef ZXING_EXPERIMENTAL_API
Barcode create_barcode(py::object content, BarcodeFormat format, std::string ec_level)
{
	auto cOpts = CreatorOptions(format).ecLevel(ec_level);
	auto data = py::cast<std::string>(content);

	if (py::isinstance<py::str>(content))
		return CreateBarcodeFromText(data, cOpts);
	else if (py::isinstance<py::bytes>(content))
		return CreateBarcodeFromBytes(data, cOpts);
	else
		throw py::type_error("Invalid input: only 'str' and 'bytes' supported.");
}

Image write_barcode_to_image(Barcode barcode, int size_hint, bool with_hrt, bool with_quiet_zones)
{
	return WriteBarcodeToImage(barcode, WriterOptions().sizeHint(size_hint).withHRT(with_hrt).withQuietZones(with_quiet_zones));
}

std::string write_barcode_to_svg(Barcode barcode, int size_hint, bool with_hrt, bool with_quiet_zones)
{
	return WriteBarcodeToSVG(barcode, WriterOptions().sizeHint(size_hint).withHRT(with_hrt).withQuietZones(with_quiet_zones));
}
#endif

Image write_barcode(BarcodeFormat format, py::object content, int width, int height, int quiet_zone, int ec_level)
{
#ifdef ZXING_EXPERIMENTAL_API
	auto barcode = create_barcode(content, format, std::to_string(ec_level));
	return write_barcode_to_image(barcode, std::max(width, height), false, quiet_zone != 0);
#else
	CharacterSet encoding [[maybe_unused]];
	if (py::isinstance<py::str>(content))
		encoding  = CharacterSet::UTF8;
	else if (py::isinstance<py::bytes>(content))
		encoding = CharacterSet::BINARY;
	else
		throw py::type_error("Invalid input: only 'str' and 'bytes' supported.");

	auto writer = MultiFormatWriter(format).setEncoding(encoding).setMargin(quiet_zone).setEccLevel(ec_level);
	auto bits = writer.encode(py::cast<std::string>(content), width, height);
	auto bitmap = ToMatrix<uint8_t>(bits);
	Image res(bitmap.width(), bitmap.height());
	memcpy(const_cast<uint8_t*>(res.data()), bitmap.data(), bitmap.size());
	return res;
#endif
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
		.value("MicroQRCode", BarcodeFormat::MicroQRCode)
		.value("RMQRCode", BarcodeFormat::RMQRCode)
		.value("DataBar", BarcodeFormat::DataBar)
		.value("DataBarExpanded", BarcodeFormat::DataBarExpanded)
		.value("DataBarLimited", BarcodeFormat::DataBarLimited)
		.value("DXFilmEdge", BarcodeFormat::DXFilmEdge)
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
			":rtype: zxingcpp.Point")
		.def_property_readonly("top_right", &Position::topRight,
			":return: coordinate of the symbol's top-right corner\n"
			":rtype: zxingcpp.Point")
		.def_property_readonly("bottom_left", &Position::bottomLeft,
			":return: coordinate of the symbol's bottom-left corner\n"
			":rtype: zxingcpp.Point")
		.def_property_readonly("bottom_right", &Position::bottomRight,
			":return: coordinate of the symbol's bottom-right corner\n"
			":rtype: zxingcpp.Point")
		.def("__str__", [](Position pos) {
			std::ostringstream oss;
			oss << pos;
			return oss.str();
		});
	py::enum_<Error::Type>(m, "ErrorType", "")
		.value("None", Error::Type::None, "No error")
		.value("Format", Error::Type::Format, "Data format error")
		.value("Checksum", Error::Type::Checksum, "Checksum error")
		.value("Unsupported", Error::Type::Unsupported, "Unsupported content error")
		.export_values();
	py::class_<Error>(m, "Error", "Barcode reading error")
		.def_property_readonly("type", &Error::type,
		   ":return: Error type\n"
		   ":rtype: zxingcpp.ErrorType")
		.def_property_readonly("message", &Error::msg,
			":return: Error message\n"
			":rtype: str")
		.def("__str__", [](Error e) { return ToString(e); });
	py::class_<Barcode>(m, "Barcode", "The Barcode class")
		.def_property_readonly("valid", &Barcode::isValid,
			":return: whether or not barcode is valid (i.e. a symbol was found and decoded)\n"
			":rtype: bool")
		.def_property_readonly("text", [](const Barcode& res) { return res.text(); },
			":return: text of the decoded symbol (see also TextMode parameter)\n"
			":rtype: str")
		.def_property_readonly("bytes", [](const Barcode& res) { return py::bytes(res.bytes().asString()); },
			":return: uninterpreted bytes of the decoded symbol\n"
			":rtype: bytes")
		.def_property_readonly("format", &Barcode::format,
			":return: decoded symbol format\n"
			":rtype: zxingcpp.BarcodeFormat")
		.def_property_readonly("symbology_identifier", &Barcode::symbologyIdentifier,
			":return: decoded symbology idendifier\n"
			":rtype: str")
		.def_property_readonly("ec_level", &Barcode::ecLevel,
			":return: error correction level of the symbol (empty string if not applicable)\n"
			":rtype: str")
		.def_property_readonly("content_type", &Barcode::contentType,
			":return: content type of symbol\n"
			":rtype: zxingcpp.ContentType")
		.def_property_readonly("position", &Barcode::position,
			":return: position of the decoded symbol\n"
			":rtype: zxingcpp.Position")
		.def_property_readonly("orientation", &Barcode::orientation,
			":return: orientation (in degree) of the decoded symbol\n"
			":rtype: int")
		.def_property_readonly(
			"error", [](const Barcode& res) { return res.error() ? std::optional(res.error()) : std::nullopt; },
			":return: Error code or None\n"
			":rtype: zxingcpp.Error")
#ifdef ZXING_EXPERIMENTAL_API
		.def("to_image", &write_barcode_to_image,
			  py::arg("size_hint") = 0,
			  py::arg("with_hrt") = false,
			  py::arg("with_quiet_zones") = true)
		.def("to_svg", &write_barcode_to_svg,
			  py::arg("size_hint") = 0,
			  py::arg("with_hrt") = false,
			  py::arg("with_quiet_zones") = true)
#endif
		;
	m.attr("Result") = m.attr("Barcode"); // alias to deprecated name for the Barcode class
	m.def("barcode_format_from_str", &BarcodeFormatFromString,
		py::arg("str"),
		"Convert string to BarcodeFormat\n\n"
		":type str: str\n"
		":param str: string representing barcode format\n"
		":return: corresponding barcode format\n"
		":rtype: zxingcpp.BarcodeFormat");
	m.def("barcode_formats_from_str", &BarcodeFormatsFromString,
		py::arg("str"),
		"Convert string to BarcodeFormats\n\n"
		":type str: str\n"
		":param str: string representing a list of barcodes formats\n"
		":return: corresponding barcode formats\n"
		":rtype: zxingcpp.BarcodeFormats");
	m.def("read_barcode", &read_barcode,
		py::arg("image"),
		py::arg("formats") = BarcodeFormats{},
		py::arg("try_rotate") = true,
		py::arg("try_downscale") = true,
		py::arg("text_mode") = TextMode::HRI,
		py::arg("binarizer") = Binarizer::LocalAverage,
		py::arg("is_pure") = false,
		py::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		py::arg("return_errors") = false,
		"Read (decode) a barcode from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: buffer|numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a buffer with the correct shape, use .cast on memory view to convert\n"
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
		":param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.\n"
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
		":type return_errors: bool\n"
		":param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.\n"
		" Default is False."
		":rtype: zxingcpp.Barcode\n"
		":return: a Barcode if found, None otherwise"
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
		py::arg("return_errors") = false,
		"Read (decode) multiple barcodes from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: buffer|numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a buffer with the correct shape, use .cast on memory view to convert\n"
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
		":param text_mode: specifies the TextMode that governs how the raw bytes content is transcoded to text.\n"
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
		":type return_errors: bool\n"
		":param return_errors: Set to True to return the barcodes with errors as well (e.g. checksum errors); see ``Barcode.error``.\n"
		" Default is False.\n"
		":rtype: list[zxingcpp.Barcode]\n"
		":return: a list of Barcodes, the list is empty if none is found"
	);
	py::class_<Image>(m, "Image", py::buffer_protocol())
		.def_property_readonly(
			"__array_interface__",
			[](const Image& m) {
				return py::dict("version"_a = 3, "data"_a = m, "shape"_a = py::make_tuple(m.height(), m.width()), "typestr"_a = "|u1");
			})
		.def_property_readonly("shape", [](const Image& m) { return py::make_tuple(m.height(), m.width()); })
		.def_buffer([](const Image& m) -> py::buffer_info {
			return {
				const_cast<uint8_t*>(m.data()),                 // Pointer to buffer
				sizeof(uint8_t),                                // Size of one scalar
				py::format_descriptor<uint8_t>::format(),       // Python struct-style format descriptor
				2,                                              // Number of dimensions
				{m.height(), m.width()},                        // Buffer dimensions
				{m.rowStride(), m.pixStride()},                 // Strides (in bytes) for each index
				true                                            // read-only
			};
		});

#ifdef ZXING_EXPERIMENTAL_API
	m.def("create_barcode", &create_barcode,
		py::arg("content"),
		py::arg("format"),
		py::arg("ec_level") = ""
	);

	m.def("write_barcode_to_image", &write_barcode_to_image,
		py::arg("barcode"),
		py::arg("size_hint") = 0,
		py::arg("with_hrt") = false,
		py::arg("with_quiet_zones") = true
	);

	m.def("write_barcode_to_svg", &write_barcode_to_svg,
		py::arg("barcode"),
		py::arg("size_hint") = 0,
		py::arg("with_hrt") = false,
		py::arg("with_quiet_zones") = true
	);
#endif

	m.attr("Bitmap") = m.attr("Image"); // alias to deprecated name for the Image class
	m.def("write_barcode", &write_barcode,
		py::arg("format"),
		py::arg("text"),
		py::arg("width") = 0,
		py::arg("height") = 0,
		py::arg("quiet_zone") = -1,
		py::arg("ec_level") = -1,
		"Write (encode) a text into a barcode and return 8-bit grayscale bitmap buffer\n\n"
		":type format: zxing.BarcodeFormat\n"
		":param format: format of the barcode to create\n"
		":type text: str|bytes\n"
		":param text: the text/content of the barcode. A str is encoded as utf8 text and bytes as binary data\n"
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
		":param ec_level: error correction level of the barcode (Used for Aztec, PDF417, and QRCode only).\n"
		":rtype: zxingcpp.Bitmap\n"
	);
}
