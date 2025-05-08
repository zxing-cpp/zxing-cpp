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
#include <bit>
#else
#include "BitMatrix.h"
#include "Matrix.h"
#include "MultiFormatWriter.h"
#include <cstring>
#endif

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <optional>
#include <sstream>
#include <vector>

using namespace ZXing;
namespace nb = nanobind;
using namespace nb::literals; // to bring in the `_a` literal

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	os.seekp(-1, os.cur);
	os << '\0';
	return os;
}

std::string ToString(nb::dlpack::dtype dt){
	std::string res;
	if (dt.code == (uint8_t) nb::dlpack::dtype_code::Bool) {
		res = "bool";
	} else {
			switch (dt.code) {
					case (uint8_t) nb::dlpack::dtype_code::Int: res = "int"; break;
					case (uint8_t) nb::dlpack::dtype_code::UInt: res = "uint"; break;
					case (uint8_t) nb::dlpack::dtype_code::Float: res = "float"; break;
					case (uint8_t) nb::dlpack::dtype_code::Complex: res = "complex"; break;
			}
			res += std::to_string(dt.bits);
	}
	return res;
}

auto read_barcodes_impl(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, TextMode text_mode,
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

	if (ImageView _imageview; nb::try_cast(_image, _imageview)) {
		// Disables the GIL during zxing processing (restored automatically upon completion)
		nb::gil_scoped_release release;
		return ReadBarcodes(_imageview, opts);
	}

	const auto _type_object = nb::type_object(_image.type(), nb::detail::borrow_t{});
	const auto _type = nb::cast<std::string>(nb::str(_type_object));
	nb::ndarray<nb::ro> arr;
	ImageFormat imgfmt = ImageFormat::None;
	try {
		if (nb::hasattr(_image, "__array_interface__")) {
			if (_type.find("PIL.") != std::string::npos) {
				_image.attr("load")();
				const auto mode = nb::cast<std::string>(_image.attr("mode"));
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

			auto ai = nb::cast<nb::dict>(_image.attr("__array_interface__"));
			auto shape = nb::cast<std::vector<size_t>>(ai["shape"]);
			auto typestr = nb::cast<std::string>(ai["typestr"]);

			if (typestr != "|u1")
				nb::raise_type_error("Incompatible __array_interface__ data type (%s): expected a uint8_t array (|u1).", typestr.c_str());

			if (ai.contains("data")) {
				auto adata = ai["data"];

				if (nb::try_cast(ai["data"], arr)) {
					// PIL and our own __array_interface__ passes data as a buffer/bytes object
					// PIL's bytes object has wrong dim/shape/strides info
					if (arr.ndim() != Size(shape))
						arr = nb::ndarray<nb::ro>{arr.data(), shape.size(), shape.data(), nb::find(arr), nullptr, nb::dtype<uint8_t>()};
				} else if (nb::tuple data_tuple; nb::try_cast(ai["data"], data_tuple)) {
					// numpy data is passed as a tuple
					auto strides = std::vector<int64_t>{};
					if (ai.contains("strides") && !ai["strides"].is_none())
						strides = nb::cast<std::vector<int64_t>>(ai["strides"]);
					auto data_ptr = reinterpret_cast<const void*>(nb::cast<uintptr_t>(nb::cast<nb::tuple>(adata)[0]));
					arr = nb::ndarray<nb::ro>{data_ptr, shape.size(), shape.data(), nb::handle(), strides.empty() ? nullptr : strides.data(), nb::dtype<uint8_t>()};
				} else if (!nb::try_cast(_image, arr)) {
					nb::raise_type_error("No way to get data from __array_interface__");
				}
			} else {
				arr = nb::cast<nb::ndarray<nb::ro>>(_image);
			}
#ifdef ZXING_EXPERIMENTAL_API
		} else if(_type.find("QtGui.QImage") != std::string::npos) {
			const std::string format = nb::cast<std::string>(nb::str(_image.attr("format")()));
			if (format.ends_with("Format_ARGB32") || format.ends_with("Format_RGB32")) {
				if constexpr (std::endian::native == std::endian::little)
					imgfmt = ImageFormat::BGRA;
				else
					imgfmt = ImageFormat::ARGB;
			} else if (format.ends_with("Format_RGBA8888"))
				imgfmt = ImageFormat::RGBA;
			else if (format.ends_with("Format_RGB888"))
				imgfmt = ImageFormat::RGB;
			else if (format.ends_with("Format_BGR888"))
				imgfmt = ImageFormat::BGR;
			else if (format.ends_with("Format_Grayscale8"))
				imgfmt = ImageFormat::Lum;
			else {
				_image = _image.attr("convertToFormat")(24); // 24 is Format_Greyscale8
				imgfmt = ImageFormat::Lum;
			}
			arr = nb::cast<nb::ndarray<nb::ro>>(_image.attr("constBits")());
			arr = nb::ndarray<nb::ro>{
					arr.data(),
					{nb::cast<size_t>(_image.attr("height")()),
					nb::cast<size_t>(_image.attr("width")()),
					static_cast<size_t>(PixStride(imgfmt))},
					nb::find(arr),
					{},
					nb::dtype<uint8_t>()};
#endif
		} else {
			arr = nb::cast<nb::ndarray<nb::ro>>(_image);
		}
	} catch (nb::python_error &e) {
		nb::raise_from(e, PyExc_TypeError, ("Invalid input: " + _type + " does not support the buffer protocol.").c_str());
	} catch (...) {
		nb::raise_type_error("Invalid input: %s does not support the buffer protocol.", _type.c_str());
	}
	if (arr.dtype() != nb::dtype<uint8_t>())
		nb::raise_type_error("Incompatible buffer format '%s': expected a uint8_t array.", ToString(arr.dtype()).c_str());

	if (arr.ndim() != 2 && arr.ndim() != 3)
		nb::raise_type_error("Incompatible buffer dimension %s (needs to be 2 or 3).", std::to_string(arr.ndim()).c_str());

	const auto height = narrow_cast<int>(arr.shape(0));
	const auto width = narrow_cast<int>(arr.shape(1));
	const auto channels = arr.ndim() == 2 ? 1 : narrow_cast<int>(arr.shape(2));
	const auto rowStride = narrow_cast<int>(arr.stride(0));
	const auto pixStride = narrow_cast<int>(arr.stride(1));
	if (imgfmt == ImageFormat::None) {
		// Assume grayscale or BGR image depending on channels number
		if (channels == 1)
			imgfmt = ImageFormat::Lum;
		else if (channels == 3)
			imgfmt = ImageFormat::BGR;
		else
			throw nb::value_error(("Unsupported number of channels for buffer: " + std::to_string(channels)).c_str());
	}

	const auto bytes = static_cast<const uint8_t*>(arr.data());
	// Disables the GIL during zxing processing (restored automatically upon completion)
	nb::gil_scoped_release release;
	return ReadBarcodes({bytes, width, height, imgfmt, rowStride, pixStride}, opts);
}

std::optional<Barcode> read_barcode(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale,
									TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol,
									bool return_errors)
{
	auto res = read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol,
								  return_errors, 1);
	return res.empty() ? std::nullopt : std::optional(res.front());
}

Barcodes read_barcodes(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, TextMode text_mode,
					   Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, bool return_errors)
{
	return read_barcodes_impl(_image, formats, try_rotate, try_downscale, text_mode, binarizer, is_pure, ean_add_on_symbol,
							  return_errors);
}

#ifdef ZXING_EXPERIMENTAL_API
Barcode create_barcode(nb::object content, BarcodeFormat format, std::string ec_level)
{
	auto cOpts = CreatorOptions(format).ecLevel(ec_level);

	if (nb::str content_str; nb::try_cast(content, content_str))
		return CreateBarcodeFromText(nb::cast<std::string>(content_str), cOpts);
	else if (nb::bytes content_bytes; nb::try_cast(content, content_bytes))
		return CreateBarcodeFromBytes(content_bytes.data(), content_bytes.size(), cOpts);
	else
		nb::raise_type_error("Invalid input: only 'str' and 'bytes' supported.");
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

Image write_barcode(BarcodeFormat format, nb::object content, int width, int height, int quiet_zone, int ec_level)
{
#ifdef ZXING_EXPERIMENTAL_API
	auto barcode = create_barcode(content, format, std::to_string(ec_level));
	return write_barcode_to_image(barcode, std::max(width, height), false, quiet_zone != 0);
#else
	BitMatrix bits;
	auto writer = MultiFormatWriter(format).setMargin(quiet_zone).setEccLevel(ec_level);
	if (nb::str content_str; nb::try_cast(content, content_str))
		bits = writer.setEncoding(CharacterSet::UTF8).encode(nb::cast<std::string>(content_str), width, height);
	else if (nb::bytes content_bytes; nb::try_cast(content, content_bytes))
		bits = writer.setEncoding(CharacterSet::BINARY).encode({content_bytes.c_str(), content_bytes.size()}, width, height);
	else
		nb::raise_type_error("Invalid input: only 'str' and 'bytes' supported.");

	auto bitmap = ToMatrix<uint8_t>(bits);
	Image res(bitmap.width(), bitmap.height());
	memcpy(const_cast<uint8_t*>(res.data()), bitmap.data(), bitmap.size());
	return res;
#endif
}

// Implementation of the buffer protocol for ImageView and Image
int ImageView_getbuffer_impl(ImageView& self, PyObject* obj, Py_buffer* view, int flags) {
	view->obj = obj;
	view->buf = const_cast<uint8_t*>(self.data());
	view->format = const_cast<char*>("B");
	view->itemsize = sizeof(uint8_t);
	view->ndim = 2;
	view->len = self.height() * self.rowStride();
	view->readonly = true;
	view->suboffsets = nullptr;
	view->internal = nullptr;

	Py_INCREF(view->obj);
	view->strides = new Py_ssize_t[2]{self.rowStride(), self.pixStride()};
	view->shape = new Py_ssize_t[2]{self.height(), self.width()};
	return 0;
}

extern "C" inline int ImageView_getbuffer(PyObject* obj, Py_buffer* view, int flags) {
	auto &self = nb::cast<ZXing::ImageView&>(nb::handle(obj));
	return ImageView_getbuffer_impl(self, obj, view, flags);
}

extern "C" inline int Image_getbuffer(PyObject* obj, Py_buffer* view, int flags) {
	auto &self = nb::cast<ZXing::Image&>(nb::handle(obj));
	return ImageView_getbuffer_impl(self, obj, view, flags);
}

extern "C" inline void releasebuffer(PyObject*, Py_buffer* view) {
	delete[] view->strides;
	delete[] view->shape;
}

NB_MODULE(zxingcpp, m)
{
	m.doc() = "python bindings for zxing-cpp";

	// forward declaration of BarcodeFormats to fix BarcodeFormat function header typings
	// see https://github.com/zxing-cpp/zxing-cpp/pull/271
	nb::class_<BarcodeFormats> pyBarcodeFormats(m, "BarcodeFormats");

	nb::enum_<BarcodeFormat>(m, "BarcodeFormat", nb::is_arithmetic{}, "Enumeration of zxing supported barcode formats")
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
		.def("__repr__", nb::overload_cast<BarcodeFormats>(static_cast<std::string(*)(BarcodeFormats)>(ToString)))
		.def("__str__", nb::overload_cast<BarcodeFormats>(static_cast<std::string(*)(BarcodeFormats)>(ToString)))
		.def("__eq__", [](BarcodeFormats f1, BarcodeFormats f2){ return f1 == f2; })
		.def("__or__", [](BarcodeFormats fs, BarcodeFormat f){ return fs | f; })
		.def(nb::init_implicit<BarcodeFormat>());
	nb::enum_<Binarizer>(m, "Binarizer", "Enumeration of binarizers used before decoding images")
		.value("BoolCast", Binarizer::BoolCast)
		.value("FixedThreshold", Binarizer::FixedThreshold)
		.value("GlobalHistogram", Binarizer::GlobalHistogram)
		.value("LocalAverage", Binarizer::LocalAverage)
		.export_values();
	nb::enum_<EanAddOnSymbol>(m, "EanAddOnSymbol", "Enumeration of options for EAN-2/5 add-on symbols check")
		.value("Ignore", EanAddOnSymbol::Ignore, "Ignore any Add-On symbol during read/scan")
		.value("Read", EanAddOnSymbol::Read, "Read EAN-2/EAN-5 Add-On symbol if found")
		.value("Require", EanAddOnSymbol::Require, "Require EAN-2/EAN-5 Add-On symbol to be present")
		.export_values();
	nb::enum_<ContentType>(m, "ContentType", "Enumeration of content types")
		.value("Text", ContentType::Text)
		.value("Binary", ContentType::Binary)
		.value("Mixed", ContentType::Mixed)
		.value("GS1", ContentType::GS1)
		.value("ISO15434", ContentType::ISO15434)
		.value("UnknownECI", ContentType::UnknownECI)
		.export_values();
	nb::enum_<TextMode>(m, "TextMode", "")
		.value("Plain", TextMode::Plain, "bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)")
		.value("ECI", TextMode::ECI, "standard content following the ECI protocol with every character set ECI segment transcoded to unicode")
		.value("HRI", TextMode::HRI, "Human Readable Interpretation (dependent on the ContentType)")
		.value("Hex", TextMode::Hex, "bytes() transcoded to ASCII string of HEX values")
		.value("Escaped", TextMode::Escaped, "Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to '<GS>'")
		.export_values();
	nb::enum_<ImageFormat>(m, "ImageFormat", "Enumeration of image formats supported by read_barcodes")
		.value("Lum", ImageFormat::Lum)
		.value("LumA", ImageFormat::LumA)
		.value("RGB", ImageFormat::RGB)
		.value("BGR", ImageFormat::BGR)
		.value("RGBA", ImageFormat::RGBA)
		.value("ARGB", ImageFormat::ARGB)
		.value("BGRA", ImageFormat::BGRA)
		.value("ABGR", ImageFormat::ABGR)
		.export_values();
	nb::class_<PointI>(m, "Point", "Represents the coordinates of a point in an image")
		.def_ro("x", &PointI::x,
			":return: horizontal coordinate of the point\n"
			":rtype: int")
		.def_ro("y", &PointI::y,
			":return: vertical coordinate of the point\n"
			":rtype: int");
	nb::class_<Position>(m, "Position", "The position of a decoded symbol")
		.def_prop_ro("top_left", &Position::topLeft,
			":return: coordinate of the symbol's top-left corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("top_right", &Position::topRight,
			":return: coordinate of the symbol's top-right corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("bottom_left", &Position::bottomLeft,
			":return: coordinate of the symbol's bottom-left corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("bottom_right", &Position::bottomRight,
			":return: coordinate of the symbol's bottom-right corner\n"
			":rtype: zxingcpp.Point")
		.def("__str__", [](Position pos) {
			std::ostringstream oss;
			oss << pos;
			return oss.str();
		});
	nb::enum_<Error::Type>(m, "ErrorType", "")
		.value("None", Error::Type::None, "No error")
		.value("Format", Error::Type::Format, "Data format error")
		.value("Checksum", Error::Type::Checksum, "Checksum error")
		.value("Unsupported", Error::Type::Unsupported, "Unsupported content error")
		.export_values();
	nb::class_<Error>(m, "Error", "Barcode reading error")
		.def_prop_ro("type", &Error::type,
		   ":return: Error type\n"
		   ":rtype: zxingcpp.ErrorType")
		.def_prop_ro("message", &Error::msg,
			":return: Error message\n"
			":rtype: str")
		.def("__str__", [](Error e) { return ToString(e); });
	nb::class_<Barcode>(m, "Barcode", "The Barcode class")
		.def_prop_ro("valid", &Barcode::isValid,
			":return: whether or not barcode is valid (i.e. a symbol was found and decoded)\n"
			":rtype: bool")
		.def_prop_ro("text", [](const Barcode& res) { return res.text(); },
			":return: text of the decoded symbol (see also TextMode parameter)\n"
			":rtype: str")
		.def_prop_ro("bytes", [](const Barcode& res) { return nb::bytes(res.bytes().data(), res.bytes().size());},
			":return: uninterpreted bytes of the decoded symbol\n"
			":rtype: bytes")
		.def_prop_ro("format", &Barcode::format,
			":return: decoded symbol format\n"
			":rtype: zxingcpp.BarcodeFormat")
		.def_prop_ro("symbology_identifier", &Barcode::symbologyIdentifier,
			":return: decoded symbology idendifier\n"
			":rtype: str")
		.def_prop_ro("ec_level", &Barcode::ecLevel,
			":return: error correction level of the symbol (empty string if not applicable)\n"
			":rtype: str")
		.def_prop_ro("content_type", &Barcode::contentType,
			":return: content type of symbol\n"
			":rtype: zxingcpp.ContentType")
		.def_prop_ro("position", &Barcode::position,
			":return: position of the decoded symbol\n"
			":rtype: zxingcpp.Position")
		.def_prop_ro("orientation", &Barcode::orientation,
			":return: orientation (in degree) of the decoded symbol\n"
			":rtype: int")
		.def_prop_ro(
			"error", [](const Barcode& res) { return res.error() ? std::optional(res.error()) : std::nullopt; },
			":return: Error code or None\n"
			":rtype: zxingcpp.Error")
#ifdef ZXING_EXPERIMENTAL_API
		.def("to_image", &write_barcode_to_image,
			  nb::arg("size_hint") = 0,
			  nb::arg("with_hrt") = false,
			  nb::arg("with_quiet_zones") = true)
		.def("to_svg", &write_barcode_to_svg,
			  nb::arg("size_hint") = 0,
			  nb::arg("with_hrt") = false,
			  nb::arg("with_quiet_zones") = true)
#endif
		;
	m.attr("Result") = m.attr("Barcode"); // alias to deprecated name for the Barcode class
	m.def("barcode_format_from_str", &BarcodeFormatFromString,
		nb::arg("str"),
		"Convert string to BarcodeFormat\n\n"
		":type str: str\n"
		":param str: string representing barcode format\n"
		":return: corresponding barcode format\n"
		":rtype: zxingcpp.BarcodeFormat");
	m.def("barcode_formats_from_str", &BarcodeFormatsFromString,
		nb::arg("str"),
		"Convert string to BarcodeFormats\n\n"
		":type str: str\n"
		":param str: string representing a list of barcodes formats\n"
		":return: corresponding barcode formats\n"
		":rtype: zxingcpp.BarcodeFormats");
	m.def("read_barcode", &read_barcode,
		nb::arg("image"),
		nb::arg("formats") = BarcodeFormats{},
		nb::arg("try_rotate") = true,
		nb::arg("try_downscale") = true,
		nb::arg("text_mode") = TextMode::HRI,
		nb::arg("binarizer") = Binarizer::LocalAverage,
		nb::arg("is_pure") = false,
		nb::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		nb::arg("return_errors") = false,
		"Read (decode) a barcode from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: buffer|numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a buffer with the correct shape, use .cast on memory view to convert\n"
		"  - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n"
		"  - a PIL Image\n"
		"  - a QtGui.QImage\n"
		"  - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat\n"
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
		nb::arg("image"),
		nb::arg("formats") = BarcodeFormats{},
		nb::arg("try_rotate") = true,
		nb::arg("try_downscale") = true,
		nb::arg("text_mode") = TextMode::HRI,
		nb::arg("binarizer") = Binarizer::LocalAverage,
		nb::arg("is_pure") = false,
		nb::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		nb::arg("return_errors") = false,
		"Read (decode) multiple barcodes from a numpy BGR or grayscale image array or from a PIL image.\n\n"
		":type image: buffer|numpy.ndarray|PIL.Image.Image\n"
		":param image: The image object to decode. The image can be either:\n"
		"  - a buffer with the correct shape, use .cast on memoryview to convert\n"
		"  - a numpy array containing image either in grayscale (1 byte per pixel) or BGR mode (3 bytes per pixel)\n"
		"  - a PIL Image\n"
		"  - a QtGui.QImage\n"
		"  - a zxingcpp.ImageView object, which is effectively a memory view but with custom strides and ImageFormat\n"
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

// Buffer protocol slot IDs, officially defined in Python 3.9+ but available since Python 3.0.
// Reference: https://github.com/python/cpython/blob/f6cdc6b4a191b75027de342aa8b5d344fb31313e/Include/typeslots.h#L2-L3
#ifndef Py_bf_getbuffer
	#define Py_bf_getbuffer 1
	#define Py_bf_releasebuffer 2
#endif

	PyType_Slot Image_slots[] = {
		{Py_bf_getbuffer, (void*)Image_getbuffer},
		{Py_bf_releasebuffer, (void*)releasebuffer},
		{0, nullptr}};

	nb::class_<Image>(m, "Image", nb::type_slots(Image_slots), nb::is_weak_referenceable())
		.def_prop_ro("__array_interface__", [](const Image& image) {
			auto d = nb::dict();
			d["version"] = 3;
			d["data"] = &image;
			d["shape"] = nb::make_tuple(image.height(), image.width());
			d["typestr"] = "|u1";
			return d;
		});

#ifdef ZXING_EXPERIMENTAL_API
	m.def("create_barcode", &create_barcode,
		nb::arg("content"),
		nb::arg("format"),
		nb::arg("ec_level") = ""
	);

	m.def("write_barcode_to_image", &write_barcode_to_image,
		nb::arg("barcode"),
		nb::arg("size_hint") = 0,
		nb::arg("with_hrt") = false,
		nb::arg("with_quiet_zones") = true
	);

	m.def("write_barcode_to_svg", &write_barcode_to_svg,
		nb::arg("barcode"),
		nb::arg("size_hint") = 0,
		nb::arg("with_hrt") = false,
		nb::arg("with_quiet_zones") = true
	);

	PyType_Slot ImageView_slots[] = {
		{Py_bf_getbuffer, (void*)ImageView_getbuffer},
		{Py_bf_releasebuffer, (void*)releasebuffer},
		{0, nullptr}};

	nb::class_<ImageView>(m, "ImageView", nb::type_slots(ImageView_slots), nb::is_weak_referenceable())
		.def("__init__",
			[](ImageView* self, nb::ndarray<nb::ro> buffer, int width, int height, ImageFormat format, int rowStride, int pixStride) {
				if (buffer.dtype() != nb::dtype<uint8_t>())
					nb::raise_type_error("Incompatible buffer format '%s': expected a uint8_t array.", ToString(buffer.dtype()).c_str());

				new (self) ImageView(static_cast<const uint8_t*>(buffer.data()), buffer.size(), width, height, format, rowStride, pixStride);
			},
			nb::arg("buffer"),
			nb::arg("width"),
			nb::arg("height"),
			nb::arg("format"),
			nb::arg("row_stride") = 0,
			nb::arg("pix_stride") = 0)
		.def_prop_ro("format", [](const ImageView& iv) { return iv.format(); });

#endif

	m.attr("Bitmap") = m.attr("Image"); // alias to deprecated name for the Image class
	m.def("write_barcode", &write_barcode,
		nb::arg("format"),
		nb::arg("text"),
		nb::arg("width") = 0,
		nb::arg("height") = 0,
		nb::arg("quiet_zone") = -1,
		nb::arg("ec_level") = -1,
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
