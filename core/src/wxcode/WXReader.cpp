// SPDX-License-Identifier: Apache-2.0

#include "WXReader.h"

#include "BinaryBitmap.h"
#include "WXConstants.h"
#include "WXDecoder.h"
#include "WXDetector.h"

#include <algorithm>
#include <vector>

namespace ZXing::WXCode {

static std::vector<Detection> DetectWithOtsu(const BinaryBitmap& image)
{
	auto source = image.luminance();
	std::vector<uint8_t> normalized;
	// The detector accepts ImageView layouts directly. Only color conversion
	// and the binary bitmap's logical inversion require a separate buffer.
	if (source.format() != ImageFormat::Lum || image.inverted()) {
		normalized.resize(static_cast<size_t>(source.width()) * source.height());
		for (int y = 0; y < source.height(); ++y)
			for (int x = 0; x < source.width(); ++x) {
				const auto* pixel = source.data(x, y);
				auto value = source.format() == ImageFormat::Lum
								 ? *pixel
								 : RGBToLum(pixel[RedIndex(source.format())], pixel[GreenIndex(source.format())],
											pixel[BlueIndex(source.format())]);
				normalized[static_cast<size_t>(y) * source.width() + x] = image.inverted() ? 255 - value : value;
			}
		source = ImageView(normalized.data(), source.width(), source.height(), ImageFormat::Lum);
	}
	return Detect(source);
}

BarcodesData Reader::read(const BinaryBitmap& image, int maxSymbols) const
{
	if (maxSymbols <= 0 || image.closed() || !IsSupportedImageSize(image.width(), image.height()))
		return {};

	std::vector<Detection> detections;
	// LocalAverage is the shared default; WX Code uses its own Otsu threshold instead.
	if (_opts.binarizer() != Binarizer::LocalAverage) {
		if (auto* matrix = image.getBitMatrix())
			detections = Detect(image.luminance(), matrix);
	} else {
		detections = DetectWithOtsu(image);
	}

	BarcodesData results;
	for (const auto& detection : detections) {
		auto payload = Decode(detection.bits);
		if (payload.empty())
			continue;

		BarcodeData result;
		result.format = BarcodeFormat::WXCode;
		result.content = Content(std::move(payload), {});
		result.position = detection.position;
		if (std::none_of(results.begin(), results.end(), [&](const auto& existing) { return existing == result; }))
			results.push_back(std::move(result));
		if (Size(results) == maxSymbols)
			break;
	}
	return results;
}

} // namespace ZXing::WXCode
