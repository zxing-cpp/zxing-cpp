/*
 * Copyright 2019 Tim Rae
 * Copyright 2021 Antoine Humbert
 * Copyright 2021 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ZXingCpp.h"
#include "ZXAlgorithms.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/vector.h>
#include <nanobind/ndarray.h>
#include <nanobind/make_iterator.h>
#include <bit>
#include <optional>
#include <vector>

using namespace ZXing;
namespace nb = nanobind;
using namespace nanobind::literals;

static void deprecation_warning(std::string_view msg)
{
	auto warnings = nb::module_::import_("warnings");
	auto builtins = nb::module_::import_("builtins");
	warnings.attr("warn")(msg, builtins.attr("DeprecationWarning"));
}

// MARK: - Buffer protocol

struct ImagePyBufferInfo {
	Py_ssize_t shape[2];
	Py_ssize_t strides[2];
};

static int image_getbuffer(PyObject* obj, Py_buffer* view, int /*flags*/)
{
	const Image& img = nb::cast<const Image&>(nb::handle(obj));
	auto* info = new ImagePyBufferInfo{{img.height(), img.width()}, {img.rowStride(), img.pixStride()}};
	view->obj        = obj; Py_INCREF(obj);
	view->buf        = const_cast<uint8_t*>(img.data());
	view->len        = img.rowStride() * img.height();
	view->readonly   = 1;
	view->itemsize   = 1;
	view->format     = (char*)"B";
	view->ndim       = 2;
	view->shape      = info->shape;
	view->strides    = info->strides;
	view->suboffsets = nullptr;
	view->internal   = info;
	return 0;
}

static void image_releasebuffer(PyObject*, Py_buffer* view)
{
	delete static_cast<ImagePyBufferInfo*>(view->internal);
}

static PyType_Slot image_slots[] = {
	{Py_bf_getbuffer,     (void*)image_getbuffer},
	{Py_bf_releasebuffer, (void*)image_releasebuffer},
	{0, nullptr}
};

struct ImageViewPyBufferInfo {
	Py_ssize_t shape[3];
	Py_ssize_t strides[3];
};

static int imageview_getbuffer(PyObject* obj, Py_buffer* view, int /*flags*/)
{
	const ImageView& iv = nb::cast<const ImageView&>(nb::handle(obj));
	auto* info = new ImageViewPyBufferInfo{
		{iv.height(), iv.width(), PixStride(iv.format())},
		{iv.rowStride(), iv.pixStride(), 1}
	};
	view->obj        = obj; Py_INCREF(obj);
	view->buf        = const_cast<uint8_t*>(iv.data());
	view->len        = iv.rowStride() * iv.height();
	view->readonly   = 1;
	view->itemsize   = 1;
	view->format     = (char*)"B";
	view->ndim       = 3;
	view->shape      = info->shape;
	view->strides    = info->strides;
	view->suboffsets = nullptr;
	view->internal   = info;
	return 0;
}

static void imageview_releasebuffer(PyObject*, Py_buffer* view)
{
	delete static_cast<ImageViewPyBufferInfo*>(view->internal);
}

static PyType_Slot imageview_slots[] = {
	{Py_bf_getbuffer,     (void*)imageview_getbuffer},
	{Py_bf_releasebuffer, (void*)imageview_releasebuffer},
	{0, nullptr}
};

// MARK: - Reader

auto read_barcodes_impl(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, bool try_invert,
						TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, bool return_errors,
						uint8_t max_number_of_symbols = 0xff)
{
	const auto opts = ReaderOptions()
		.formats(formats)
		.tryRotate(try_rotate)
		.tryDownscale(try_downscale)
		.tryInvert(try_invert)
		.textMode(text_mode)
		.binarizer(binarizer)
		.isPure(is_pure)
		.maxNumberOfSymbols(max_number_of_symbols)
		.eanAddOnSymbol(ean_add_on_symbol)
		.returnErrors(return_errors);

	if (nb::isinstance<ImageView>(_image)) {
		nb::gil_scoped_release release;
		return ReadBarcodes(nb::cast<ImageView>(_image), opts);
	}

	const auto _type = std::string(nb::str(_image.type()).c_str());

	auto c_strides = [](const std::vector<Py_ssize_t>& shape, Py_ssize_t itemsize) {
		std::vector<Py_ssize_t> strides(shape.size());
		Py_ssize_t s = itemsize;
		for (int i = static_cast<int>(shape.size()) - 1; i >= 0; --i) {
			strides[i] = s;
			s *= shape[i];
		}
		return strides;
	};

	void* data_ptr = nullptr;
	int ndim = 0;
	std::vector<Py_ssize_t> shape;
	std::vector<Py_ssize_t> strides;
	ImageFormat imgfmt = ImageFormat::None;
	Py_buffer buf_view = {};
	bool has_buf = false;

	auto release_buf = [&] { if (has_buf) { PyBuffer_Release(&buf_view); has_buf = false; } };

	try {
		if (nb::hasattr(_image, "__array_interface__")) {
			if (_type.find("PIL.") != std::string::npos) {
				_image.attr("load")();
				const auto mode = nb::cast<std::string>(_image.attr("mode"));
				if (mode == "L")         imgfmt = ImageFormat::Lum;
				else if (mode == "RGB")  imgfmt = ImageFormat::RGB;
				else if (mode == "RGBA") imgfmt = ImageFormat::RGBA;
				else { _image = _image.attr("convert")("L"); imgfmt = ImageFormat::Lum; }
			}

			auto ai      = nb::cast<nb::dict>(_image.attr("__array_interface__"));
			shape        = nb::cast<std::vector<Py_ssize_t>>(ai["shape"]);
			auto typestr = nb::cast<std::string>(ai["typestr"]);

			if (typestr != "|u1")
				throw nb::type_error(("Incompatible __array_interface__ data type (" + typestr
									  + "): expected a uint8_t array (|u1).").c_str());

			if (ai.contains("data")) {
				auto adata = nb::cast<nb::object>(ai["data"]);
				if (nb::isinstance<nb::tuple>(adata))
					throw nb::type_error("numpy arrays must use the read_barcode(ndarray) overload");
				// PIL passes data as a bytes/buffer object
				if (PyObject_GetBuffer(adata.ptr(), &buf_view, PyBUF_SIMPLE) < 0)
					throw nb::type_error("No way to get data from __array_interface__");
				has_buf  = true;
				data_ptr = buf_view.buf;
				ndim     = static_cast<int>(shape.size());
				strides  = c_strides(shape, 1);
			} else {
				if (PyObject_GetBuffer(_image.ptr(), &buf_view, PyBUF_STRIDES | PyBUF_FORMAT) < 0)
					throw nb::type_error(("Invalid input: " + _type + " does not support the buffer protocol.").c_str());
				has_buf  = true;
				data_ptr = buf_view.buf;
				ndim     = buf_view.ndim;
				shape.assign(buf_view.shape,   buf_view.shape   + ndim);
				strides.assign(buf_view.strides, buf_view.strides + ndim);
			}
		} else if (_type.find("QtGui.QImage") != std::string::npos) {
			const std::string format = nb::cast<std::string>(nb::str(_image.attr("format")()));
			if (format.ends_with("Format_ARGB32") || format.ends_with("Format_RGB32"))
				imgfmt = std::endian::native == std::endian::little ? ImageFormat::BGRA : ImageFormat::ARGB;
			else if (format.ends_with("Format_RGBA8888"))   imgfmt = ImageFormat::RGBA;
			else if (format.ends_with("Format_RGB888"))     imgfmt = ImageFormat::RGB;
			else if (format.ends_with("Format_BGR888"))     imgfmt = ImageFormat::BGR;
			else if (format.ends_with("Format_Grayscale8")) imgfmt = ImageFormat::Lum;
			else { _image = _image.attr("convertToFormat")(24); imgfmt = ImageFormat::Lum; }

			nb::object bits = _image.attr("constBits")();
			if (PyObject_GetBuffer(bits.ptr(), &buf_view, PyBUF_SIMPLE) < 0)
				throw nb::type_error("QtGui.QImage: cannot get buffer from constBits()");
			has_buf  = true;
			data_ptr = buf_view.buf;
			ndim     = 3;
			shape    = {nb::cast<Py_ssize_t>(_image.attr("height")()), nb::cast<Py_ssize_t>(_image.attr("width")()), PixStride(imgfmt)};
			strides  = {nb::cast<Py_ssize_t>(_image.attr("bytesPerLine")()), PixStride(imgfmt), 1};
		} else {
			if (PyObject_GetBuffer(_image.ptr(), &buf_view, PyBUF_STRIDES | PyBUF_FORMAT) < 0)
				throw nb::type_error(("Invalid input: " + _type + " does not support the buffer protocol.").c_str());
			has_buf  = true;
			data_ptr = buf_view.buf;
			ndim     = buf_view.ndim;
			shape.assign(buf_view.shape,   buf_view.shape   + ndim);
			strides.assign(buf_view.strides, buf_view.strides + ndim);
		}
	} catch (nb::python_error& e) {
		release_buf();
		nb::raise_from(e, PyExc_TypeError, "%s",
					   ("Invalid input: " + _type + " does not support the buffer protocol.").c_str());
		throw;
	} catch (...) {
		release_buf();
		throw;
	}

	if (has_buf && buf_view.format && std::string_view(buf_view.format) != "B") {
		auto fmt = std::string(buf_view.format);
		release_buf();
		throw nb::type_error(("Incompatible buffer format '" + fmt + "': expected a uint8_t array.").c_str());
	}

	if (ndim != 2 && ndim != 3) {
		release_buf();
		throw nb::type_error(("Incompatible buffer dimension " + std::to_string(ndim) + " (needs to be 2 or 3).").c_str());
	}

	const auto height    = narrow_cast<int>(shape[0]);
	const auto width     = narrow_cast<int>(shape[1]);
	const auto channels  = ndim == 2 ? 1 : narrow_cast<int>(shape[2]);
	const auto rowStride = narrow_cast<int>(strides[0]);
	const auto pixStride = narrow_cast<int>(strides[1]);

	if (imgfmt == ImageFormat::None) {
		if (channels == 1)      imgfmt = ImageFormat::Lum;
		else if (channels == 3) imgfmt = ImageFormat::BGR;
		else {
			release_buf();
			throw nb::value_error(("Unsupported number of channels for buffer: " + std::to_string(channels)).c_str());
		}
	}

	const auto* bytes = static_cast<const uint8_t*>(data_ptr);
	Barcodes result;
	{
		nb::gil_scoped_release release;
		result = ReadBarcodes({bytes, width, height, imgfmt, rowStride, pixStride}, opts);
	}
	release_buf();
	return result;
}

std::optional<Barcode> read_barcode(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale,
									bool try_invert, TextMode text_mode, Binarizer binarizer, bool is_pure,
									EanAddOnSymbol ean_add_on_symbol, bool return_errors)
{
	auto res = read_barcodes_impl(_image, formats, try_rotate, try_downscale, try_invert, text_mode, binarizer, is_pure,
								  ean_add_on_symbol, return_errors, 1);
	return res.empty() ? std::nullopt : std::optional(res.front());
}

Barcodes read_barcodes(nb::object _image, const BarcodeFormats& formats, bool try_rotate, bool try_downscale, bool try_invert,
					   TextMode text_mode, Binarizer binarizer, bool is_pure, EanAddOnSymbol ean_add_on_symbol, bool return_errors)
{
	return read_barcodes_impl(_image, formats, try_rotate, try_downscale, try_invert, text_mode, binarizer, is_pure, ean_add_on_symbol,
							  return_errors);
}

// MARK: - Writer

ImageView image_view(nb::object buffer, int width, int height, ImageFormat format, int rowStride, int pixStride)
{
	Py_buffer buf_view = {};
	if (PyObject_GetBuffer(buffer.ptr(), &buf_view, PyBUF_SIMPLE | PyBUF_FORMAT) < 0)
		throw nb::type_error("ImageView: argument does not support the buffer protocol.");
	if (buf_view.format && std::string_view(buf_view.format) != "B") {
		auto fmt = std::string(buf_view.format);
		PyBuffer_Release(&buf_view);
		throw nb::type_error(("Incompatible buffer format '" + fmt + "': expected a uint8_t array.").c_str());
	}
	auto result = ImageView(static_cast<const uint8_t*>(buf_view.buf), buf_view.len,
							width, height, format, rowStride, pixStride);
	PyBuffer_Release(&buf_view);
	return result;
}

Barcode create_barcode(nb::object content, BarcodeFormat format, const nb::kwargs& kwargs)
{
	auto cOpts = CreatorOptions(format, std::string(nb::str(nb::cast<nb::object>(kwargs)).c_str()));

	if (nb::isinstance<nb::str>(content))
		return CreateBarcodeFromText(nb::cast<std::string>(content), cOpts);
	else if (nb::isinstance<nb::bytes>(content)) {
		auto b = nb::cast<nb::bytes>(content);
		return CreateBarcodeFromBytes(std::string(b.c_str(), b.size()), cOpts);
	} else
		throw nb::type_error("Invalid input: only 'str' and 'bytes' supported.");
}

Image write_barcode_to_image(Barcode barcode, int scale, bool add_hrt, bool add_quiet_zones)
{
	return WriteBarcodeToImage(barcode, WriterOptions().scale(scale).addHRT(add_hrt).addQuietZones(add_quiet_zones));
}

std::string write_barcode_to_svg(Barcode barcode, int scale, bool add_hrt, bool add_quiet_zones)
{
	return WriteBarcodeToSVG(barcode, WriterOptions().scale(scale).addHRT(add_hrt).addQuietZones(add_quiet_zones));
}

Image write_barcode(BarcodeFormat format, nb::object content, int width, int height, int quiet_zone, int ec_level)
{
	deprecation_warning("write_barcode() is deprecated, use create_barcode() and write_barcode_to_image() instead.");

#ifdef ZXING_USE_ZINT
	ec_level = format & BarcodeFormat::QRCode ? ec_level / 2 : ec_level;
#endif

	nb::dict kw_dict;
	kw_dict["ec_level"] = ec_level;
	auto cOpts = CreatorOptions(format, std::string(nb::str(nb::cast<nb::object>(kw_dict)).c_str()));

	Barcode barcode;
	if (nb::isinstance<nb::str>(content))
		barcode = CreateBarcodeFromText(nb::cast<std::string>(content), cOpts);
	else if (nb::isinstance<nb::bytes>(content)) {
		auto b = nb::cast<nb::bytes>(content);
		barcode = CreateBarcodeFromBytes(std::string(b.c_str(), b.size()), cOpts);
	} else
		throw nb::type_error("Invalid input: only 'str' and 'bytes' supported.");

	return write_barcode_to_image(barcode, -std::max(width, height), false, quiet_zone != 0);
}

// MARK: - Python

NB_MODULE(zxingcpp, m)
{
	m.doc() = "python bindings for zxing-cpp";

	// forward declaration of BarcodeFormats to fix BarcodeFormat function header typings
	nb::class_<BarcodeFormats> pyBarcodeFormats(m, "BarcodeFormats");

// MARK: - Enums

	nb::enum_<BarcodeFormat>(m, "BarcodeFormat", nb::is_arithmetic(), "Enumeration of zxing supported barcode formats")
#define X(NAME, SYM, VAR, FLAGS, ZINT, ENABLED, HRI) .value(#NAME, BarcodeFormat::NAME)
		ZX_BCF_LIST(X)
#undef X
		.value("NONE", BarcodeFormat::None)
		.value("DataBarExpanded", BarcodeFormat::DataBarExp)
		.value("DataBarLimited", BarcodeFormat::DataBarLtd)
		.value("LinearCodes", BarcodeFormat::AllLinear)
		.value("MatrixCodes", BarcodeFormat::AllMatrix)
		.export_values()
		.def("__or__", [](BarcodeFormat f1, BarcodeFormat f2) {
			deprecation_warning("operator | is deprecated, pass array or tuple instead.");
			return BarcodeFormats(f1 | f2);
		})
		.def("__str__", [](BarcodeFormat f) { return ToString(f); })
		.def_prop_ro("symbology", [](BarcodeFormat f) { return Symbology(f); });

	pyBarcodeFormats
		.def("__repr__", [](const BarcodeFormats& f) { return ToString(f); })
		.def("__eq__",   [](const BarcodeFormats& f1, const BarcodeFormats& f2) { return f1 == f2; })
		.def("__or__",
			 [](const BarcodeFormats& fs, BarcodeFormat f) {
				 deprecation_warning("operator | is deprecated, pass array or tuple instead.");
				 auto res = std::vector(fs.begin(), fs.end());
				 res.push_back(f);
				 return BarcodeFormats(std::move(res));
			 })
		.def("__len__",  [](const BarcodeFormats& fs) { return fs.size(); })
		.def("__iter__",
			 [](const BarcodeFormats& fs) {
				 return nb::make_iterator(nb::type<BarcodeFormats>(), "BarcodeFormatsIterator",
										  fs.begin(), fs.end());
			 },
			 nb::keep_alive<0, 1>())
		.def("__getitem__",
			 [](const BarcodeFormats& fs, Py_ssize_t idx) {
				 if (idx < 0) idx += static_cast<Py_ssize_t>(fs.size());
				 if (idx < 0 || idx >= static_cast<Py_ssize_t>(fs.size()))
					 throw nb::index_error("BarcodeFormats index out of range");
				 return *(fs.begin() + idx);
			 })
		.def(nb::init<BarcodeFormat>())
		.def("__init__", [](BarcodeFormats* t, nb::iterable values) {
			std::vector<BarcodeFormat> list;
			for (auto fmt : values)
				list.push_back(nb::cast<BarcodeFormat>(fmt));
			new (t) BarcodeFormats(std::move(list));
		});
	nb::implicitly_convertible<nb::list, BarcodeFormats>();
	nb::implicitly_convertible<nb::tuple, BarcodeFormats>();
	nb::implicitly_convertible<BarcodeFormat, BarcodeFormats>();

	nb::enum_<Binarizer>(m, "Binarizer", "Enumeration of binarizers used before decoding images")
		.value("BoolCast",        Binarizer::BoolCast)
		.value("FixedThreshold",  Binarizer::FixedThreshold)
		.value("GlobalHistogram", Binarizer::GlobalHistogram)
		.value("LocalAverage",    Binarizer::LocalAverage)
		.export_values();

	nb::enum_<EanAddOnSymbol>(m, "EanAddOnSymbol", "Enumeration of options for EAN-2/5 add-on symbols check")
		.value("Ignore",  EanAddOnSymbol::Ignore,  "Ignore any Add-On symbol during read/scan")
		.value("Read",    EanAddOnSymbol::Read,    "Read EAN-2/EAN-5 Add-On symbol if found")
		.value("Require", EanAddOnSymbol::Require, "Require EAN-2/EAN-5 Add-On symbol to be present")
		.export_values();

	nb::enum_<ContentType>(m, "ContentType", "Enumeration of content types")
		.value("Text",       ContentType::Text)
		.value("Binary",     ContentType::Binary)
		.value("Mixed",      ContentType::Mixed)
		.value("GS1",        ContentType::GS1)
		.value("ISO15434",   ContentType::ISO15434)
		.value("UnknownECI", ContentType::UnknownECI)
		.export_values();

	nb::enum_<TextMode>(m, "TextMode", "")
		.value("Plain",   TextMode::Plain,   "bytes() transcoded to unicode based on ECI info or guessed charset (the default mode prior to 2.0)")
		.value("ECI",     TextMode::ECI,     "standard content following the ECI protocol with every character set ECI segment transcoded to unicode")
		.value("HRI",     TextMode::HRI,     "Human Readable Interpretation (dependent on the ContentType)")
		.value("Escaped", TextMode::Escaped, "Use the EscapeNonGraphical() function (e.g. ASCII 29 will be transcoded to '<GS>'")
		.value("Hex",     TextMode::Hex,     "bytes() transcoded to ASCII string of HEX values")
		.value("HexECI",  TextMode::HexECI,  "bytesECI() transcoded to ASCII string of HEX values")
		.export_values();

	nb::enum_<ImageFormat>(m, "ImageFormat", "Enumeration of image formats supported by read_barcodes")
		.value("Lum",  ImageFormat::Lum)
		.value("LumA", ImageFormat::LumA)
		.value("RGB",  ImageFormat::RGB)
		.value("BGR",  ImageFormat::BGR)
		.value("RGBA", ImageFormat::RGBA)
		.value("ARGB", ImageFormat::ARGB)
		.value("BGRA", ImageFormat::BGRA)
		.value("ABGR", ImageFormat::ABGR)
		.export_values();

// MARK: - Classes

	nb::class_<PointI>(m, "Point", "Represents the coordinates of a point in an image")
		.def_ro("x", &PointI::x,
			":return: horizontal coordinate of the point\n"
			":rtype: int")
		.def_ro("y", &PointI::y,
			":return: vertical coordinate of the point\n"
			":rtype: int");

	nb::class_<Position>(m, "Position", "The position of a decoded symbol")
		.def_prop_ro("top_left",     &Position::topLeft,
			":return: coordinate of the symbol's top-left corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("top_right",    &Position::topRight,
			":return: coordinate of the symbol's top-right corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("bottom_left",  &Position::bottomLeft,
			":return: coordinate of the symbol's bottom-left corner\n"
			":rtype: zxingcpp.Point")
		.def_prop_ro("bottom_right", &Position::bottomRight,
			":return: coordinate of the symbol's bottom-right corner\n"
			":rtype: zxingcpp.Point")
		.def("__str__", [](Position pos) { return ToString(pos); });

	nb::enum_<Error::Type>(m, "ErrorType", "")
		.value("None",        Error::Type::None,        "No error")
		.value("Format",      Error::Type::Format,      "Data format error")
		.value("Checksum",    Error::Type::Checksum,    "Checksum error")
		.value("Unsupported", Error::Type::Unsupported, "Unsupported content error")
		.export_values();

	nb::class_<Error>(m, "Error", "Barcode reading error")
		.def_prop_ro("type",    &Error::type,
		   ":return: Error type\n"
		   ":rtype: zxingcpp.ErrorType")
		.def_prop_ro("message", &Error::msg,
			":return: Error message\n"
			":rtype: str")
		.def("__str__", [](Error e) { return ToString(e); });

	nb::class_<Barcode>(m, "Barcode", "The Barcode class", nb::dynamic_attr())
		.def_prop_ro("valid",  &Barcode::isValid,
			":return: whether or not barcode is valid (i.e. a symbol was found and decoded)\n"
			":rtype: bool")
		.def_prop_ro("text",   [](const Barcode& res) { return res.text(); },
			":return: text of the decoded symbol (see also TextMode parameter)\n"
			":rtype: str")
		.def_prop_ro("bytes",  [](const Barcode& res) {
			return nb::bytes((const char*)res.bytes().data(), res.bytes().size());
		},
			":return: uninterpreted bytes of the decoded symbol\n"
			":rtype: bytes")
		.def_prop_ro("format",               &Barcode::format,
			":return: decoded symbol format\n"
			":rtype: zxingcpp.BarcodeFormat")
		.def_prop_ro("symbology",            &Barcode::symbology,
			":return: decoded symbol symbology\n"
			":rtype: zxingcpp.BarcodeFormat")
		.def_prop_ro("symbology_identifier", &Barcode::symbologyIdentifier,
			":return: decoded symbology identifier\n"
			":rtype: str")
		.def_prop_ro("ec_level",             &Barcode::ecLevel,
			":return: error correction level of the symbol (empty string if not applicable)\n"
			":rtype: str")
		.def_prop_ro("content_type",         &Barcode::contentType,
			":return: content type of symbol\n"
			":rtype: zxingcpp.ContentType")
		.def_prop_ro("position",             &Barcode::position,
			":return: position of the decoded symbol\n"
			":rtype: zxingcpp.Position")
		.def_prop_ro("orientation",          &Barcode::orientation,
			":return: orientation (in degree) of the decoded symbol\n"
			":rtype: int")
		.def_prop_ro("error", [](const Barcode& res) {
			return res.error() ? std::optional(res.error()) : std::nullopt;
		},
			":return: Error code or None\n"
			":rtype: zxingcpp.Error")
		.def_prop_ro("extra", [](nb::object self) -> nb::object {
			if (nb::hasattr(self, "_cached_extra"))
				return self.attr("_cached_extra");
			const auto extra = nb::cast<const Barcode&>(self).extra();
			if (extra.empty()) {
				self.attr("_cached_extra") = nb::none();
			} else {
				try {
					auto json   = nb::module_::import_("json");
					auto parsed = json.attr("loads")(extra);
					self.attr("_cached_extra") = parsed;
				} catch (nb::python_error& e) {
					throw nb::value_error(
						(std::string("Invalid JSON in Barcode::extra(): ") + e.what()).c_str());
				}
			}
			return self.attr("_cached_extra");
		},
			":return: Symbology specific extra information as a Python dictionary (might be empty)\n"
			":rtype: dict")
		.def("to_image", &write_barcode_to_image,
			 nb::arg("scale") = 1, nb::arg("add_hrt") = false, nb::arg("add_quiet_zones") = true)
		.def("to_svg",   &write_barcode_to_svg,
			 nb::arg("scale") = 1, nb::arg("add_hrt") = false, nb::arg("add_quiet_zones") = true);
	m.attr("Result") = m.attr("Barcode");

// MARK: - Functions (writer helpers and barcode creation)

	nb::class_<Image>(m, "Image", nb::type_slots(image_slots))
		.def_prop_ro("__array_interface__",
			[](nb::object self) {
				const Image& img = nb::cast<const Image&>(self);
				nb::dict d;
				d["version"] = 3;
				d["data"]    = self;
				d["shape"]   = nb::make_tuple(img.height(), img.width());
				d["typestr"] = "|u1";
				return d;
			})
		.def_prop_ro("shape", [](const Image& img) {
			return nb::make_tuple(img.height(), img.width());
		});

	m.def("create_barcode", &create_barcode);

	m.def("write_barcode_to_image", &write_barcode_to_image,
		nb::arg("barcode"),
		nb::arg("scale")           = 1,
		nb::arg("add_hrt")         = false,
		nb::arg("add_quiet_zones") = true);

	m.def("write_barcode_to_svg", &write_barcode_to_svg,
		nb::arg("barcode"),
		nb::arg("scale")           = 1,
		nb::arg("add_hrt")         = false,
		nb::arg("add_quiet_zones") = true);

	nb::class_<ImageView>(m, "ImageView", nb::type_slots(imageview_slots))
		.def("__init__", [](ImageView* iv, nb::object buffer, int width, int height, ImageFormat format,
							int row_stride, int pix_stride) {
			new (iv) ImageView(image_view(buffer, width, height, format, row_stride, pix_stride));
		},
			 nb::arg("buffer"),
			 nb::arg("width"),
			 nb::arg("height"),
			 nb::arg("format"),
			 nb::arg("row_stride") = 0,
			 nb::arg("pix_stride") = 0)
		.def_prop_ro("format", [](const ImageView& iv) { return iv.format(); });

	m.attr("Bitmap") = m.attr("Image");

// MARK: - Functions (readers)

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

	m.def("barcode_formats_list", &BarcodeFormats::list,
		nb::arg("filter") = BarcodeFormats{},
		"Returns a list of available/supported barcode formats, optionally filtered by the provided format(s).\n\n"
		":type filter: zxingcpp.BarcodeFormats\n"
		":param filter: the BarcodeFormat(s) to filter by\n"
		":return: list of available/supported barcode formats (optionally filtered)\n"
		":rtype: list[zxingcpp.BarcodeFormat]");

	m.def("read_barcode", &read_barcode,
		nb::arg("image"),
		nb::arg("formats")           = BarcodeFormats{},
		nb::arg("try_rotate")        = true,
		nb::arg("try_downscale")     = true,
		nb::arg("try_invert")        = true,
		nb::arg("text_mode")         = TextMode::HRI,
		nb::arg("binarizer")         = Binarizer::LocalAverage,
		nb::arg("is_pure")           = false,
		nb::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		nb::arg("return_errors")     = false,
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
		":type try_invert: bool\n"
		":param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.\n"
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
		nb::arg("formats")           = BarcodeFormats{},
		nb::arg("try_rotate")        = true,
		nb::arg("try_downscale")     = true,
		nb::arg("try_invert")        = true,
		nb::arg("text_mode")         = TextMode::HRI,
		nb::arg("binarizer")         = Binarizer::LocalAverage,
		nb::arg("is_pure")           = false,
		nb::arg("ean_add_on_symbol") = EanAddOnSymbol::Ignore,
		nb::arg("return_errors")     = false,
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
		":type try_invert: bool\n"
		":param try_invert: if ``True`` (the default), decoder also tries inverted (light on dark) barcodes.\n"
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

	m.def("write_barcode", &write_barcode,
		nb::arg("format"),
		nb::arg("text"),
		nb::arg("width")      = 0,
		nb::arg("height")     = 0,
		nb::arg("quiet_zone") = -1,
		nb::arg("ec_level")   = -1,
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
