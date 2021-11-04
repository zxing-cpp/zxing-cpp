#pragma once
/*
 * Copyright 2021 Axel Waggershauser
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ReadBarcode.h"
#include "TextUtfEncoding.h"

#include <opencv2/opencv.hpp>

inline ZXing::ImageView ImageViewFromMat(const cv::Mat& image)
{
	using ZXing::ImageFormat;

	auto fmt = ImageFormat::None;
	switch (image.channels()) {
	case 1: fmt = ImageFormat::Lum; break;
	case 3: fmt = ImageFormat::BGR; break;
	case 4: fmt = ImageFormat::BGRX; break;
	}

	if (image.depth() != CV_8U || fmt == ImageFormat::None)
		return {nullptr, 0, 0, ImageFormat::None};

	return {image.data, image.cols, image.rows, fmt};
}

inline ZXing::Result ReadBarcode(const cv::Mat& image, const ZXing::DecodeHints& hints = {})
{
	return ZXing::ReadBarcode(ImageViewFromMat(image), hints);
}

inline void DrawResult(cv::Mat& img, ZXing::Result res)
{
	auto pos = res.position();
	auto zx2cv = [](ZXing::PointI p) { return cv::Point(p.x, p.y); };
	auto contour = std::vector<cv::Point>{zx2cv(pos[0]), zx2cv(pos[1]), zx2cv(pos[2]), zx2cv(pos[3])};
	const auto* pts = contour.data();
	int npts = contour.size();

	cv::polylines(img, &pts, &npts, 1, true, CV_RGB(0, 255, 0));
	cv::putText(img, ZXing::TextUtfEncoding::ToUtf8(res.text()), cv::Point(10, 20), cv::FONT_HERSHEY_DUPLEX, 0.5,
				CV_RGB(0, 255, 0));
}
