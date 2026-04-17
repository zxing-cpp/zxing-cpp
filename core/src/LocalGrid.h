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

/**
 * @brief Represents a local grid for sampling and analyzing regions within a BitMatrix image.
 *
 * The LocalGrid class tries to find the center of a module and step vectors that point to the next module in x and y direction.
 * It provides methods to get the position of a point in the grid and to sample values from the image at those positions.
 * It supports searching for timing pattern crosses and specific (alignment) patterns within the grid, which updates the origin and
 * step vectors accordingly.
 */
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

	/**
	 * @brief Constructs a LocalGrid object for mapping a region of a BitMatrix using a perspective transform.
	 *
	 * @param image The source BitMatrix representing the image data.
	 * @param mod2Pix The (global) PerspectiveTransform used to map module coordinates to pixel coordinates.
	 * @param p The logical position of the grid origin in module coordinates.
	 * @param dim The dimensions (width and height) of the (global) grid in modules.
	 * @param offset Optional offset in modules to specify the starting position of the search for the module center.
	 */
	LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI p, PointI dim, PointI offset = {});

	inline PointF getPos(PointF p = {}) const { return origin + p.x * stepX + p.y * stepY; }
	inline PointF getPos(PointI p) const { return getPos(PointF(p.x, p.y)); }

	inline Value get(PointF p) const
	{
		auto q = getPos(p);
		log(q, 4);
		return img->isIn(q) ? Value{img->get(q)} : Value{};
	}

	inline Value get(PointI p) const { return get(PointF(p.x, p.y)); }

	/**
	 * @brief Finds the center of a timing pattern cross in the local grid.
	 *
	 * This function searches for the center point of a timing pattern cross, which is a key feature in Aztec code detection, within a
	 * specified radius.
	 *
	 * @param isBlack Indicates whether the central module of the timing pattern cross is expected to be black or white.
	 * @param radius The length of the timing pattern cross arms.
	 * @return An optional PointF representing the center of the timing pattern cross if found; std::nullopt otherwise.
	 */
	std::optional<PointF> findTimingPatternCross(bool isBlack, int radius);

	using Directions = std::span<const PointI>;

	/**
	 * @brief Searches for a specific pattern within a grid based on provided parameters.
	 *
	 * This function attempts to locate a pattern in the grid by starting from given points
	 * and following specified directions for timing, black, and white modules.
	 */
	bool findPattern(int radius, PointI timingStart, Directions timingDirs, PointI blackStart, Directions blackDirs, PointI whiteStart,
					 Directions whiteDirs);
};

} // namespace ZXing
