/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2020 Axel Waggershauser
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
	LogMatrixWriter lmw(log, image, 5, "grid.pnm");
#endif
	auto isInside = [&](PointI p) { return image.isIn(mod2Pix(centered(p))); };

	if (width <= 0 || height <= 0 || !mod2Pix.isValid() || !isInside({0, 0}) || !isInside({width - 1, 0}) ||
		!isInside({width - 1, height - 1}) || !isInside({0, height - 1}))
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
