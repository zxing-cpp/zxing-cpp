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

	void adjustOriginAndStep(PointF& dir, int radius, std::span<const PointF> offsets);
	bool isTimingPatternCross(PointI p, bool isBlack, int radius, int errorThreshold = 0);
	bool findValue(PointI p, PointI dir, Value v)
	{
		// check if there is a value v at p or a quarter module before or after p or p is outside the image
		auto d = PointF(dir.x ? 0.25 : 0.0, dir.y ? 0.25 : 0.0);
		return !img->isIn(getPos(p)) || get(p) == v || get(p + d) == v || get(p - d) == v;
	};

public:
	LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI p, PointI dim);
	
	inline PointF getPos(PointF p = {}) const { return origin + p.x * stepX + p.y * stepY; }
	inline PointF getPos(PointI p) const { return getPos(PointF(p.x, p.y)); }

	inline Value get(PointF p) const
	{
		auto q = getPos(p);
		log(q, 4);
		return img->isIn(q) ? Value{img->get(q)} : Value{};
	}

	inline Value get(PointI p) const { return get(PointF(p.x, p.y)); }

	std::optional<PointF> findTimingPatternCross(bool isBlack, int radius);
};

} // namespace ZXing
