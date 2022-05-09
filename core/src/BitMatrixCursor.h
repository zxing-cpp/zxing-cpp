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

#include <array>

namespace ZXing {

enum class Direction { LEFT = -1, RIGHT = 1 };

inline Direction opposite(Direction dir) noexcept
{
	return dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
}

/**
 * @brief The BitMatrixCursor represents a current position inside an image and current direction it can advance towards.
 *
 * The current position and direction is a PointT<T>. So depending on the type it can be used to traverse the image
 * in a Bresenham style (PointF) or in a discrete way (step only horizontal/vertical/diagonal (PointI)).
 */
template<typename POINT>
class BitMatrixCursor
{
public:
	const BitMatrix* img;

	POINT p; // current position
	POINT d; // current direction

	BitMatrixCursor(const BitMatrix& image, POINT p, POINT d) : img(&image), p(p) { setDirection(d); }

	class Value
	{
		enum { INVALID = -1, WHITE = 0, BLACK = 1 };
		int v = INVALID;
	public:
		Value() = default;
		Value(bool isBlack) : v(isBlack) {}
		bool isValid() const noexcept { return v != INVALID; }
		bool isWhite() const noexcept { return v == WHITE; }
		bool isBlack() const noexcept { return v == BLACK; }

		operator bool() const noexcept { return isValid(); }

		bool operator==(Value o) const { return v == o.v; }
		bool operator!=(Value o) const { return v != o.v; }
	};

	template <typename T>
	Value testAt(PointT<T> p) const
	{
		return img->isIn(p) ? Value{img->get(p)} : Value{};
	}

	bool blackAt(POINT pos) const noexcept { return testAt(pos).isBlack(); }
	bool whiteAt(POINT pos) const noexcept { return testAt(pos).isWhite(); }

	bool isIn(POINT p) const noexcept { return img->isIn(p); }
	bool isIn() const noexcept { return isIn(p); }
	bool isBlack() const noexcept { return blackAt(p); }
	bool isWhite() const noexcept { return whiteAt(p); }

	POINT front() const noexcept { return d; }
	POINT back() const noexcept { return {-d.x, -d.y}; }
	POINT left() const noexcept { return {d.y, -d.x}; }
	POINT right() const noexcept { return {-d.y, d.x}; }
	POINT direction(Direction dir) const noexcept { return static_cast<int>(dir) * right(); }

	void turnBack() noexcept { d = back(); }
	void turnLeft() noexcept { d = left(); }
	void turnRight() noexcept { d = right(); }
	void turn(Direction dir) noexcept { d = direction(dir); }

	Value edgeAt(POINT d) const noexcept
	{
		Value v = testAt(p);
		return testAt(p + d) != v ? v : Value();
	}

	Value edgeAtFront() const noexcept { return edgeAt(front()); }
	Value edgeAtBack() const noexcept { return edgeAt(back()); }
	Value edgeAtLeft() const noexcept { return edgeAt(left()); }
	Value edgeAtRight() const noexcept { return edgeAt(right()); }
	Value edgeAt(Direction dir) const noexcept { return edgeAt(direction(dir)); }

	void setDirection(PointF dir) { d = bresenhamDirection(dir); }
	void setDirection(PointI dir) { d = dir; }

	bool step(typename POINT::value_t s = 1)
	{
		p += s * d;
		return isIn(p);
	}

	BitMatrixCursor<POINT> movedBy(POINT d) const
	{
		auto res = *this;
		res.p += d;
		return res;
	}

	/**
	 * @brief stepToEdge advances cursor to one step behind the next (or n-th) edge.
	 * @param nth number of edges to pass
	 * @param range max number of steps to take
	 * @param backup whether or not to backup one step so we land in front of the edge
	 * @return number of steps taken or 0 if moved outside of range/image
	 */
	int stepToEdge(int nth = 1, int range = 0, bool backup = false)
	{
		// TODO: provide an alternative and faster out-of-bounds check than isIn() inside testAt()
		int steps = 0;
		auto lv = testAt(p);

		while (nth && (!range || steps < range) && lv.isValid()) {
			++steps;
			auto v = testAt(p + steps * d);
			if (lv != v) {
				lv = v;
				--nth;
			}
		}
		if (backup)
			--steps;
		p += steps * d;
		return steps * (nth == 0);
	}

	bool stepAlongEdge(Direction dir, bool skipCorner = false)
	{
		if (!edgeAt(dir))
			turn(dir);
		else if (edgeAtFront()) {
			turn(opposite(dir));
			if (edgeAtFront()) {
				turn(opposite(dir));
				if (edgeAtFront())
					return false;
			}
		}

		bool ret = step();

		if (ret && skipCorner && !edgeAt(dir)) {
			turn(dir);
			ret = step();
		}

		return ret;
	}

	int countEdges(int range = 0)
	{
		int res = 0;

		while (int steps = stepToEdge(1, range)) {
			range -= steps;
			++res;
		}

		return res;
	}

	template<typename ARRAY>
	ARRAY readPattern(int range = 0)
	{
		ARRAY res;
		for (auto& i : res)
			i = stepToEdge(1, range);
		return res;
	}

	template<typename ARRAY>
	ARRAY readPatternFromBlack(int maxWhitePrefix, int range = 0)
	{
		if (maxWhitePrefix && isWhite() && !stepToEdge(1, maxWhitePrefix))
			return {};
		return readPattern<ARRAY>(range);
	}
};

using BitMatrixCursorF = BitMatrixCursor<PointF>;
using BitMatrixCursorI = BitMatrixCursor<PointI>;

} // ZXing
