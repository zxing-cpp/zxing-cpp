#include "BarcodeFormat.h"

// Reader
#include "ReadBarcode.h"

// Writer
#include "BitMatrix.h"
#include "MultiFormatWriter.h"
#include "TextUtfEncoding.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <memory>
#include <vector>

using namespace ZXing;
namespace py = pybind11;

// Numpy array wrapper class for images (either BGR or GRAYSCALE)
using Image = py::array_t<uint8_t, py::array::c_style>;

template<typename OUT, typename IN>
OUT narrow(IN in)
{
	return static_cast<OUT>(in);
}

std::ostream& operator<<(std::ostream& os, const Position& points) {
	for (const auto& p : points)
		os << p.x << "x" << p.y << " ";
	os.seekp(-1, os.cur);
	os << '\0';
	return os;
}

Result read_barcode(const Image& image, const BarcodeFormats& formats, bool fastMode, bool tryRotate, Binarizer binarizer)
{
	DecodeHints hints;
	hints.setTryHarder(!fastMode);
	hints.setTryRotate(tryRotate);
	hints.setFormats(formats);
	hints.setBinarizer(binarizer);
	const auto height = narrow<int>(image.shape(0));
	const auto width = narrow<int>(image.shape(1));
	const auto channels = image.ndim() == 2 ? 1 : narrow<int>(image.shape(2));
	const auto bytes = image.data();
	const auto imgfmt = channels == 1 ? ImageFormat::Lum : ImageFormat::BGR;

	return ReadBarcode({bytes, width, height, imgfmt, width * channels, channels}, hints);
}

Image write_barcode(BarcodeFormat format, std::string text, int width, int height, int margin, int eccLevel)
{
	auto writer = MultiFormatWriter(format).setMargin(margin).setEccLevel(eccLevel);
	auto bitmap = writer.encode(TextUtfEncoding::FromUtf8(text), width, height);

	auto result = Image({bitmap.height(), bitmap.width()});
	auto r = result.mutable_unchecked<2>();
	for (ssize_t y = 0; y < r.shape(0); y++)
		for (ssize_t x = 0; x < r.shape(1); x++)
			r(y, x) = bitmap.get(narrow<int>(x), narrow<int>(y)) ? 0 : 255;
	return result;
}

PYBIND11_MODULE(zxing, m)
{
	m.doc() = "python bindings for zxing-cpp";
	py::enum_<BarcodeFormat>(m, "BarcodeFormat", py::arithmetic{})
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
		.value("DataBar", BarcodeFormat::DataBar)
		.value("DataBarExpanded", BarcodeFormat::DataBarExpanded)
		.value("UPCA", BarcodeFormat::UPCA)
		.value("UPCE", BarcodeFormat::UPCE)
		// use upper case 'NONE' because 'None' is a reserved identifyer in python
		.value("NONE", BarcodeFormat::None)
		.value("OneDCodes", BarcodeFormat::OneDCodes)
		.value("TwoDCodes", BarcodeFormat::TwoDCodes)
		.export_values()
		// see https://github.com/pybind/pybind11/issues/2221
		.def("__or__", [](BarcodeFormat f1, BarcodeFormat f2){ return f1 | f2; })
		.def("__or__", [](BarcodeFormats fs, BarcodeFormat f){ return fs | f; });
	py::class_<BarcodeFormats>(m, "BarcodeFormats")
		.def("__repr__", py::overload_cast<BarcodeFormats>(&ToString))
		.def("__str__", py::overload_cast<BarcodeFormats>(&ToString))
		.def("__eq__", [](BarcodeFormats f1, BarcodeFormats f2){ return f1 == f2; })
		.def(py::init<BarcodeFormat>());
	py::implicitly_convertible<BarcodeFormat, BarcodeFormats>();
	py::enum_<Binarizer>(m, "Binarizer")
		.value("BoolCast", Binarizer::BoolCast)
		.value("FixedThreshold", Binarizer::FixedThreshold)
		.value("GlobalHistogram", Binarizer::GlobalHistogram)
		.value("LocalAverage", Binarizer::LocalAverage)
		.export_values();
	py::class_<PointI>(m, "Point")
		.def_readonly("x", &PointI::x)
		.def_readonly("y", &PointI::y);
	py::class_<Position>(m, "Position")
		.def_property_readonly("topLeft", &Position::topLeft)
		.def_property_readonly("topRight", &Position::topRight)
		.def_property_readonly("bottomLeft", &Position::bottomLeft)
		.def_property_readonly("bottomRight", &Position::bottomRight)
		.def("__str__", [](Position pos) {
			std::ostringstream oss;
			oss << pos;
			return oss.str();
		});
	py::class_<Result>(m, "Result")
		.def_property_readonly("valid", &Result::isValid)
		.def_property_readonly("text", &Result::text)
		.def_property_readonly("format", &Result::format)
		.def_property_readonly("position", &Result::position)
		.def_property_readonly("orientation", &Result::orientation);
	m.def("barcode_format_from_str", &BarcodeFormatFromString, "Convert string to BarcodeFormat", py::arg("str"));
	m.def("barcode_formats_from_str", &BarcodeFormatsFromString, "Convert string to BarcodeFormat", py::arg("str"));
	m.def("read_barcode", &read_barcode, "Read (decode) a barcode from a numpy BGR or grayscale image array",
		py::arg("image"),
		py::arg("formats") = BarcodeFormats{},
		py::arg("fastMode") = false,
		py::arg("tryRotate") = true,
		py::arg("binarizer") = Binarizer::LocalAverage
	);
	m.def("write_barcode", &write_barcode, "Write (encode) a text into a barcode and return numpy image array",
		py::arg("format"),
		py::arg("text"),
		py::arg("width") = 0,
		py::arg("height") = 0,
		py::arg("margin") = -1,
		py::arg("eccLevel") = -1
	);
}
