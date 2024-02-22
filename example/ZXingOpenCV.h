/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ReadBarcode.h"

#include <opencv2/opencv.hpp>

inline ZXing::ImageView ImageViewFromMat(const cv::Mat& image)
{
	using ZXing::ImageFormat;

	auto fmt = ImageFormat::None;
	switch (image.channels()) {
	case 1: fmt = ImageFormat::Lum; break;
	case 2: fmt = ImageFormat::LumA; break;
	case 3: fmt = ImageFormat::BGR; break;
	case 4: fmt = ImageFormat::BGRA; break;
	}

	if (image.depth() != CV_8U || fmt == ImageFormat::None)
		return {nullptr, 0, 0, ImageFormat::None};

	return {image.data, image.cols, image.rows, fmt};
}

inline ZXing::Barcodes ReadBarcodes(const cv::Mat& image, const ZXing::ReaderOptions& options = {})
{
	return ZXing::ReadBarcodes(ImageViewFromMat(image), options);
}

inline void DrawBarcode(cv::Mat& img, ZXing::Barcode barcode)
{
	auto pos = barcode.position();
	auto zx2cv = [](ZXing::PointI p) { return cv::Point(p.x, p.y); };
	auto contour = std::vector<cv::Point>{zx2cv(pos[0]), zx2cv(pos[1]), zx2cv(pos[2]), zx2cv(pos[3])};
	const auto* pts = contour.data();
	int npts = contour.size();

	cv::polylines(img, &pts, &npts, 1, true, CV_RGB(0, 255, 0));
	cv::putText(img, barcode.text(), zx2cv(pos[3]) + cv::Point(0, 20), cv::FONT_HERSHEY_DUPLEX, 0.5, CV_RGB(0, 255, 0));
}
