// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BitMatrixCursor.h"
#include "BitMatrix.h"
#include "Log.h"
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
public:
	const BitMatrix* img;
	const PerspectiveTransform& mod2Pix;
	PointI dim, center;
	PointF origin, stepX, stepY;

	enum class Axis { X, Y };

	// dir: bresenham direction of the step vector, modSize: mod size in steps along dir, limit: max. steps when searching for an edge

	// estimate the module size from edge-to-edge distances; returns nullopt if there is no (significant) improvement
	std::optional<double> estimateModSize(PointF dir, double modSize, int limit, int last, std::span<const PointI> offsets) const;
	// estimate the shift of the origin along dir from the distances to the nearest edges
	std::optional<double> estimateOriginShift(PointF dir, double modSize, int limit, int first, int last,
											  std::span<const PointI> offsets, bool trimToGrid) const;
	bool adjustOriginAndStep(Axis axis, int first, int last, std::span<const PointI> offsets, bool trimToGrid = false);
	bool isTimingPatternCross(PointI p, bool isBlack, int radius, int errorThreshold = 0);

public:

	using Value = BitMatrixCursorF::Value;

	/**
	 * @brief Constructs a LocalGrid object for mapping a region of a BitMatrix using a perspective transform.
	 *
	 * @param image The source BitMatrix representing the image data.
	 * @param mod2Pix The (global) PerspectiveTransform used to map module coordinates to pixel coordinates.
	 * @param dim The dimensions (width and height) of the (global) grid in modules.
	 */
	LocalGrid(const BitMatrix& image, const PerspectiveTransform& mod2Pix, PointI dim) : img(&image), mod2Pix(mod2Pix), dim(dim)
	{}

	LocalGrid(const LocalGrid& other) = default;
	void operator=(const LocalGrid& other)
	{
		assert(&mod2Pix == &other.mod2Pix && img == other.img && dim == other.dim);
		center = other.center;
		origin = other.origin;
		stepX = other.stepX;
		stepY = other.stepY;
	}

	/**
	 * @brief Sets/finds the origin of the local grid near the specified module coordinates.
	 *
	 * @param p The logical position of the grid origin in module coordinates.
	 * @param offset Optional offset in modules to specify the starting position of the search for the module center.
	 */
	LocalGrid& at(PointI p, PointI offset = {});

	inline void moveBy(PointI offset)
	{
		origin = getPos(offset);
		center += offset;
	}

	bool tuneFromTiming(PointI start, std::string_view dirs, int radius); // returns true if the origin and step were successfully adjusted

	inline PointF getPos(PointF p = {}) const { return origin + p.x * stepX + p.y * stepY; }
	inline PointF getPos(PointI p) const { return getPos(PointF(p.x, p.y)); }

	inline Value get(PointF p) const
	{
		auto q = getPos(p);
		// log(q, LOG_GR);
		return img->isIn(q) ? Value{img->get(q)} : Value{};
	}

	inline Value get(PointI p) const { return get(PointF(p.x, p.y)); }

	inline bool findValue(PointI p, int r, PointI dir, Value v)
	{
		// check if there is a value v at p or a quarter module before or after p or p is outside the image
		auto d = PointF(dir.x ? 0.25 : 0.0, dir.y ? 0.25 : 0.0);
		p = p + r * dir;
		log(getPos(p), LOG_GR);
#if 1
		return get(p) == v || get(p + d) == v || get(p - d) == v;
#else
		return get(p) == v;
#endif
	}

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

	enum Pattern
	{
		B = 0b11,
		W = 0b00,
		BW = 0b01,
		WB = 0b10
	};

	// {"lr", BW, {1, 0}, 4}
	struct LinePattern
	{
		std::string_view dirs;
		Pattern pattern;
		PointI start = {};
		int last = 0, first = 0;

		bool isTiming() const { return pattern == BW || pattern == WB; }
	};

	/**
	 * @brief Searches for a specific pattern within a grid based on provided parameters.
	 *
	 * This function attempts to locate a pattern in the grid by starting from given points
	 * and following specified directions for timing, black, and white modules.
	 */
	std::optional<PointF> find(std::span<const LinePattern> patterns, int radius = 0);
	std::optional<PointF> find(std::initializer_list<LinePattern> patterns, int radius = 0)
	{
		return find(std::span(patterns), radius);
	}

	std::optional<PointF> findCorner(int radius, PointI out);
};

} // namespace ZXing
