/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"

#include "ZXAlgorithms.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace ZXing {

struct BarcodeFormatName
{
	BarcodeFormat format;
	std::string_view name;
};

static const BarcodeFormatName NAMES[] = {
	{BarcodeFormat::None, "None"},
	{BarcodeFormat::Aztec, "Aztec"},
	{BarcodeFormat::Codabar, "Codabar"},
	{BarcodeFormat::Code39, "Code39"},
	{BarcodeFormat::Code93, "Code93"},
	{BarcodeFormat::Code128, "Code128"},
	{BarcodeFormat::DataBar, "DataBar"},
	{BarcodeFormat::DataBarExpanded, "DataBarExpanded"},
	{BarcodeFormat::DataBarLimited, "DataBarLimited"},
	{BarcodeFormat::DataMatrix, "DataMatrix"},
	{BarcodeFormat::DXFilmEdge, "DXFilmEdge"},
	{BarcodeFormat::EAN8, "EAN-8"},
	{BarcodeFormat::EAN13, "EAN-13"},
	{BarcodeFormat::ITF, "ITF"},
	{BarcodeFormat::MaxiCode, "MaxiCode"},
	{BarcodeFormat::MicroQRCode, "MicroQRCode"},
	{BarcodeFormat::PDF417, "PDF417"},
	{BarcodeFormat::QRCode, "QRCode"},
	{BarcodeFormat::RMQRCode, "rMQRCode"},
	{BarcodeFormat::UPCA, "UPC-A"},
	{BarcodeFormat::UPCE, "UPC-E"},
	{BarcodeFormat::LinearCodes, "Linear-Codes"},
	{BarcodeFormat::MatrixCodes, "Matrix-Codes"},
};

std::string ToString(BarcodeFormat format)
{
	auto i = FindIf(NAMES, [format](auto& v) { return v.format == format; });
	return i == std::end(NAMES) ? std::string() : std::string(i->name);
}

std::string ToString(BarcodeFormats formats)
{
	if (formats.empty())
		return ToString(BarcodeFormat::None);
	std::string res;
	for (auto f : formats)
		res += ToString(f) + "|";
	return res.substr(0, res.size() - 1);
}

BarcodeFormat BarcodeFormatFromString(std::string_view str)
{
	auto i = FindIf(NAMES, [str](auto& v) { return IsEqualIgnoreCaseAnd(v.name, str, "_-"); });
	return i == std::end(NAMES) ? BarcodeFormat::None : i->format;
}

BarcodeFormats BarcodeFormatsFromString(std::string_view str)
{
	BarcodeFormats res;
	ForEachToken(TrimWS(str, " []"), " ,|", [&res](std::string_view token) {
		if (!token.empty()) {
			auto bc = BarcodeFormatFromString(token);
			if (bc == BarcodeFormat::None)
				throw std::invalid_argument("This is not a valid barcode format: '" + std::string(token) + "'");
			res |= bc;
		}
	});
	return res;
}

} // ZXing
