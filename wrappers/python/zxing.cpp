#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>
#include <string>
#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "Result.h"
#include "TextUtfEncoding.h"

#if 0
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
using Binarizer = ZXing::HybridBinarizer;
#endif

namespace py = pybind11;

// Note: Image is a Numpy array in BGR format (the default opencv format)
using Image = py::array_t<uint8_t, py::array::c_style>;

std::string decode(const Image& image, bool fastMode, bool tryRotate, ZXing::BarcodeFormat format) {
	ZXing::DecodeHints hints;
	hints.setShouldTryHarder(!fastMode);
	hints.setShouldTryRotate(tryRotate);
	if (format != ZXing::BarcodeFormat::FORMAT_COUNT) {
		hints.setPossibleFormats({ format });
	}
	ZXing::MultiFormatReader reader(hints);
	const auto height = image.shape(0);
	const auto width = image.shape(1);
	const auto channels = image.shape(2);
	const auto bytes = image.data();
	ZXing::GenericLuminanceSource source(width, height, bytes, width*channels, channels, 2, 1, 0);
	Binarizer binImage(std::shared_ptr<ZXing::LuminanceSource>(&source, [](void*) {}));
	const auto result = reader.read(binImage);
        if (result.isValid()) {
		return ZXing::TextUtfEncoding::ToUtf8(result.text());
	} else {
		return "";
	}
}

PYBIND11_MODULE(zxingcpp, m) {
	m.doc() = "python bindings for zxingcpp";
	py::enum_<ZXing::BarcodeFormat>(m, "BarcodeFormat")
		.value("AZTEC", ZXing::BarcodeFormat::AZTEC)
		.value("CODABAR", ZXing::BarcodeFormat::CODABAR)
		.value("CODE_39", ZXing::BarcodeFormat::CODE_39)
		.value("CODE_93", ZXing::BarcodeFormat::CODE_93)
		.value("CODE_128", ZXing::BarcodeFormat::CODE_128)
		.value("DATA_MATRIX", ZXing::BarcodeFormat::DATA_MATRIX)
		.value("EAN_8", ZXing::BarcodeFormat::EAN_8)
		.value("EAN_13", ZXing::BarcodeFormat::EAN_13)
		.value("ITF", ZXing::BarcodeFormat::ITF)
		.value("MAXICODE", ZXing::BarcodeFormat::MAXICODE)
		.value("PDF_417", ZXing::BarcodeFormat::PDF_417)
		.value("QR_CODE", ZXing::BarcodeFormat::QR_CODE)
		.value("RSS_14", ZXing::BarcodeFormat::RSS_14)
		.value("RSS_EXPANDED", ZXing::BarcodeFormat::RSS_EXPANDED)
		.value("UPC_A", ZXing::BarcodeFormat::UPC_A)
		.value("UPC_E", ZXing::BarcodeFormat::UPC_E)
		.value("UPC_EAN_EXTENSION", ZXing::BarcodeFormat::UPC_EAN_EXTENSION)
		.value("FORMAT_COUNT", ZXing::BarcodeFormat::FORMAT_COUNT)
		.export_values();
	m.def("decode", &decode, "Decode a barcode from a numpy BGR image array",
		py::arg("image"),
		py::arg("fastMode")=false,
		py::arg("tryRoate")=true,
		py::arg("format")=ZXing::BarcodeFormat::FORMAT_COUNT
	);
}

/*
	py::class_<Result>(m, "Result")
		.def(py::init<const std::string &>())
		.def("setName", &Pet::setName)
		.def("getName", &Pet::getName);
*/
