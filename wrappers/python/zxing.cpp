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

std::string decode(const Image& image, bool fastMode, bool tryRotate, std::string singleFormat) {

	ZXing::DecodeHints hints;
	hints.setShouldTryHarder(!fastMode);
	hints.setShouldTryRotate(tryRotate);
	const auto format = ZXing::BarcodeFormatFromString(singleFormat);
	if(format == ZXing::BarcodeFormat::FORMAT_COUNT && !singleFormat.empty()) {
		throw std::invalid_argument("Invalid format code: " + singleFormat);
	}
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
	m.def("decode", &decode, "Decode a barcode from a numpy BGR image array",
		py::arg("image"),
		py::arg("fastMode")=false,
		py::arg("tryRoate")=true,
		py::arg("format")=""
	);
}
