/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "GridSampler.h"

#ifdef PRINT_DEBUG
#include "LogMatrix.h"
#include "BitMatrixIO.h"
#endif

namespace ZXing {

#ifdef PRINT_DEBUG
LogMatrix log;
#endif

DetectorResult SampleGrid(const BitMatrix& image, int width, int height, const PerspectiveTransform& mod2Pix)
{
#ifdef PRINT_DEBUG
	LogMatrix log;
	static int i = 0;
	LogMatrixWriter lmw(log, image, 5, "grid" + std::to_string(i++) + ".pnm");
#endif
	if (width <= 0 || height <= 0 || !mod2Pix.isValid())
		return {};

	// To deal with remaining examples (see #251 and #267) of "numercial instabilities" that have not been
	// prevented with the Quadrilateral.h:IsConvex() check, we check for all boundary points of the grid to
	// be inside.
	auto isInside = [&](PointI p) { return image.isIn(mod2Pix(centered(p))); };
	for (int y = 0; y < height; ++y)
		if (!isInside({0, y}) || !isInside({width - 1, y}))
			return {};
	for (int x = 1; x < width - 1; ++x)
		if (!isInside({x, 0}) || !isInside({x, height - 1}))
			return {};

	BitMatrix res(width, height);
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x) {
			auto p = mod2Pix(centered(PointI{x, y}));
#ifdef PRINT_DEBUG
			log(p, 3);
#endif
			if (image.get(p))
				res.set(x, y);
		}

#ifdef PRINT_DEBUG
	printf("width: %d, height: %d\n", width, height);
	printf("%s", ToString(res).c_str());
#endif

	auto projectCorner = [&](PointI p) { return PointI(mod2Pix(PointF(p)) + PointF(0.5, 0.5)); };
	return {
		std::move(res),
		{projectCorner({0, 0}), projectCorner({width, 0}), projectCorner({width, height}), projectCorner({0, height})}};
}

} // ZXing
