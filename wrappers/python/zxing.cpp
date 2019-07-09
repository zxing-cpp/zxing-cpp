#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <iostream>
#include <string>
#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "Result.h"

using namespace ZXing;
namespace py = pybind11;

// Numpy array wrapper class for images (either BGR or GRAYSCALE)
using Image = py::array_t<uint8_t, py::array::c_style>;

Result decode(const Image& image, std::vector<BarcodeFormat> formats, bool fastMode, bool tryRotate, bool hybridBinarizer) {
	DecodeHints hints;
	hints.setShouldTryHarder(!fastMode);
	hints.setShouldTryRotate(tryRotate);
	if (formats.size()>0) {
		hints.setPossibleFormats(formats);
	}
	MultiFormatReader reader(hints);
	const auto height = image.shape(0);
	const auto width = image.shape(1);
	const auto bytes = image.data();
	std::shared_ptr<LuminanceSource> source;

	if (image.ndim() == 2) {
		// Grayscale image
		source = std::make_shared<GenericLuminanceSource>(width, height, bytes, width);
	} else {
		// BGR image
		const auto channels = image.shape(2);
		source = std::make_shared<GenericLuminanceSource>(width, height, bytes, width*channels, channels, 2, 1, 0);
	}

	if (hybridBinarizer) {
		return reader.read(HybridBinarizer(source));
	} else {
		return reader.read(GlobalHistogramBinarizer(source));
	}
}

Result decode(const Image& image, BarcodeFormat format, bool fastMode, bool tryRotate, bool hybridBinarizer) {
	if (format != BarcodeFormat::FORMAT_COUNT) {
		return decode(image, std::vector<BarcodeFormat>({format}), fastMode, tryRotate, hybridBinarizer);
	}
	else {
		return decode(image, std::vector<BarcodeFormat>({}), fastMode, tryRotate, hybridBinarizer);
	}
}

PYBIND11_MODULE(zxing, m) {
	m.doc() = "python bindings for zxing-cpp";
	py::enum_<BarcodeFormat>(m, "BarcodeFormat")
		.value("AZTEC", BarcodeFormat::AZTEC)
		.value("CODABAR", BarcodeFormat::CODABAR)
		.value("CODE_39", BarcodeFormat::CODE_39)
		.value("CODE_93", BarcodeFormat::CODE_93)
		.value("CODE_128", BarcodeFormat::CODE_128)
		.value("DATA_MATRIX", BarcodeFormat::DATA_MATRIX)
		.value("EAN_8", BarcodeFormat::EAN_8)
		.value("EAN_13", BarcodeFormat::EAN_13)
		.value("ITF", BarcodeFormat::ITF)
		.value("MAXICODE", BarcodeFormat::MAXICODE)
		.value("PDF_417", BarcodeFormat::PDF_417)
		.value("QR_CODE", BarcodeFormat::QR_CODE)
		.value("RSS_14", BarcodeFormat::RSS_14)
		.value("RSS_EXPANDED", BarcodeFormat::RSS_EXPANDED)
		.value("UPC_A", BarcodeFormat::UPC_A)
		.value("UPC_E", BarcodeFormat::UPC_E)
		.value("UPC_EAN_EXTENSION", BarcodeFormat::UPC_EAN_EXTENSION)
		.value("FORMAT_COUNT", BarcodeFormat::FORMAT_COUNT)
		.export_values();
	py::class_<ResultPoint>(m, "ResultPoint")
		.def_property_readonly("x", &ResultPoint::x)
		.def_property_readonly("y", &ResultPoint::y);
	py::class_<Result>(m, "Result")
		.def_property_readonly("valid", &Result::isValid)
		.def_property_readonly("text", &Result::text)
		.def_property_readonly("format", &Result::format)
		.def_property_readonly("points", &Result::resultPoints);
	m.def("decode", (Result (*)(const Image&, std::vector<BarcodeFormat>, bool, bool, bool))&decode, "Decode a barcode from a numpy BGR or grayscale image array",
		py::arg("image"),
		py::arg("format")=std::vector<BarcodeFormat>({}),
		py::arg("fastMode")=false,
		py::arg("tryRoate")=true,
		py::arg("hybridBinarizer")=true
	).def("decode", (Result (*)(const Image&, BarcodeFormat, bool, bool, bool))&decode, "Decode a barcode from a numpy BGR or grayscale image array",
		py::arg("image"),
		py::arg("format")=BarcodeFormat::FORMAT_COUNT,
		py::arg("fastMode")=false,
		py::arg("tryRoate")=true,
		py::arg("hybridBinarizer")=true
	);
}
