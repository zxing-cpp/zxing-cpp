#pragma once
/*
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

#include "BitMatrix.h"
#include "Matrix.h"
#include "Point.h"

#include <cassert>
#include <cstdint>

namespace ZXing {

class LogMatrix
{
	using LogBuffer = Matrix<uint8_t>;
	LogBuffer _log;
	const BitMatrix* _image = nullptr;

public:
	void init(const BitMatrix* image_)
	{
		_image = image_;
		_log = LogBuffer(_image->width(), _image->height());
	}

	void write(const char* fn)
	{
		assert(_image);

		FILE* f = fopen(fn, "wb");

		// Write PPM header, P5 == grey, P6 == rgb
		fprintf(f, "P6\n%d %d\n255\n", _image->width(), _image->height());

		// Write pixels
		for (int y = 0; y < _image->height(); ++y)
			for (int x = 0; x < _image->width(); ++x) {
				unsigned char r, g, b;
				r = g = b = _image->get(x, y) ? 0 : 255;
				switch (_log.get(x, y)) {
				case 1: r = g = b = r ? 230 : 50; break;
				case 2: r = b = 50, g = 220; break;
				case 3: g = r = 100, b = 250; break;
				}
				fwrite(&r, 1, 1, f);
				fwrite(&g, 1, 1, f);
				fwrite(&b, 1, 1, f);
			}
		fclose(f);
	}

	void operator()(const PointI& p, int color = 1)
	{
		if (0 <= p.x && 0 <= p.y && p.x < _log.width() && p.y < _log.height())
			_log.set(p.x, p.y, color);
	}

	void operator()(const std::vector<PointI>& points, int color = 2)
	{
		for (auto p : points)
			operator()(p, color);
	}
};

#ifndef NDEBUG
extern std::vector<PointI> theGrid;
#endif

} // namespace ZXing
