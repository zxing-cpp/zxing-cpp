/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
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
#include "PerspectiveTransform.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"

namespace ZXing {

namespace {

/**
* <p>Checks a set of points that have been transformed to sample points on an image against
* the image's dimensions to see if the point are even within the image.</p>
*
* <p>This method will actually "nudge" the endpoints back onto the image if they are found to be
* barely (less than 1 pixel) off the image. This accounts for imperfect detection of finder
* patterns in an image where the QR Code runs all the way to the image border.</p>
*
* <p>For efficiency, the method will check points from either end of the line until one is found
* to be within the image. Because the set of points are assumed to be linear, this is valid.</p>
*
* @param image image into which the points should map
* @param points actual points in x1,y1,...,xn,yn form
* @throws NotFoundException if an endpoint is lies outside the image boundaries
*/
static bool CheckAndNudgePoints(const BitMatrix& image, std::vector<float>& points)
{
	int width = image.width();
	int height = image.height();
	// Check and nudge points from start until we see some that are OK:
	bool nudged = true;
	for (size_t offset = 0; offset < points.size() && nudged; offset += 2) {
		int x = (int)points[offset];
		int y = (int)points[offset + 1];
		if (x < -1 || x > width || y < -1 || y > height) {
			return false;
		}
		nudged = false;
		if (x == -1) {
			points[offset] = 0.0f;
			nudged = true;
		}
		else if (x == width) {
			points[offset] = static_cast<float>(width - 1);
			nudged = true;
		}
		if (y == -1) {
			points[offset + 1] = 0.0f;
			nudged = true;
		}
		else if (y == height) {
			points[offset + 1] = static_cast<float>(height - 1);
			nudged = true;
		}
	}
	// Check and nudge points from end:
	nudged = true;
	for (int offset = int(points.size()) - 2; offset >= 0 && nudged; offset -= 2) {
		int x = (int)points[offset];
		int y = (int)points[offset + 1];
		if (x < -1 || x > width || y < -1 || y > height) {
			return false;
		}
		nudged = false;
		if (x == -1) {
			points[offset] = 0.0f;
			nudged = true;
		}
		else if (x == width) {
			points[offset] = static_cast<float>(width - 1);
			nudged = true;
		}
		if (y == -1) {
			points[offset + 1] = 0.0f;
			nudged = true;
		}
		else if (y == height) {
			points[offset + 1] = static_cast<float>(height - 1);
			nudged = true;
		}
	}
	return true;
}

class DefaultGridSampler : public GridSampler
{
public:

	BitMatrix sampleGrid(const BitMatrix& image, int dimensionX, int dimensionY,
		float p1ToX, float p1ToY, float p2ToX, float p2ToY, float p3ToX, float p3ToY, float p4ToX, float p4ToY,
		float p1FromX, float p1FromY, float p2FromX, float p2FromY, float p3FromX, float p3FromY, float p4FromX,
		float p4FromY) const override
	{
		auto transform = PerspectiveTransform::QuadrilateralToQuadrilateral(
			p1ToX, p1ToY, p2ToX, p2ToY, p3ToX, p3ToY, p4ToX, p4ToY,
			p1FromX, p1FromY, p2FromX, p2FromY, p3FromX, p3FromY, p4FromX, p4FromY);

		return sampleGrid(image, dimensionX, dimensionY, transform);
	}

	BitMatrix sampleGrid(const BitMatrix& image, int dimensionX, int dimensionY, const PerspectiveTransform& transform) const override
	{
		if (dimensionX <= 0 || dimensionY <= 0)
			return {};

		BitMatrix result(dimensionX, dimensionY);
		int max = 2 * dimensionX;
		std::vector<float> points(max);
		for (int y = 0; y < dimensionY; y++) {
			float iValue = (float)y + 0.5f;
			for (int x = 0; x < max; x += 2) {
				points[x] = static_cast<float>(x / 2) + 0.5f;
				points[x + 1] = iValue;
			}
			transform.transformPoints(points.data(), max);
			// Quick check to see if points transformed to something inside the image;
			// sufficient to check the endpoints
			if (!CheckAndNudgePoints(image, points))
				return {};
			try {
				for (int x = 0; x < max; x += 2) {
					if (image.get(static_cast<int>(points[x]), static_cast<int>(points[x + 1]))) {
						// Black(-ish) pixel
						result.set(x / 2, y);
					}
				}
			}
			catch (const std::out_of_range& aioobe) {
				// This feels wrong, but, sometimes if the finder patterns are misidentified, the resulting
				// transform gets "twisted" such that it maps a straight line of points to a set of points
				// whose endpoints are in bounds, but others are not. There is probably some mathematical
				// way to detect this about the transformation that I don't know yet.
				// This results in an ugly runtime exception despite our clever checks above -- can't have
				// that. We could check each point's coordinates but that feels duplicative. We settle for
				// catching and wrapping ArrayIndexOutOfBoundsException.
				return {};
			}
		}
		return result;
	}
};

} // anonymous

static std::shared_ptr<GridSampler> globalInstance = std::make_shared<DefaultGridSampler>();

std::shared_ptr<GridSampler>
GridSampler::Instance()
{
	return globalInstance;
}

void
GridSampler::SetInstance(const std::shared_ptr<GridSampler>& inst)
{
	globalInstance = inst;
}

} // ZXing
