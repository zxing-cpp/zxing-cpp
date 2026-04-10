// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitMatrixCursor.h"
#include "BitMatrix.h"
#include "LogMatrix.h"
#include "Point.h"
#include "PerspectiveTransform.h"

#include <optional>
#include <span>

namespace ZXing {

class LocalGrid
{
	const BitMatrix* img;
	PointF origin, stepX, stepY;
	PointI dim, center;

	using Value = BitMatrixCursorF::Value;

	void adjustOriginAndStep(PointF& dir, int radius, const std::span<const PointF> offsets, double clusterThreshold = 0);
	bool isTimingPatternCross(PointI p, int radius, int errorThreshold = 0);

public:
	LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI p, PointI dim);
	
	inline Value get(double x, double y) const
	{
		auto q = origin + x * stepX + y * stepY;
		log(q, 4);
		return img->isIn(q) ? Value{img->get(q)} : Value{};
	}

	std::optional<PointF> findTimingPatternCross(int radius);
};

} // namespace ZXing
