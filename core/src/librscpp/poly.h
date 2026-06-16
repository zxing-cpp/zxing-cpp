// Copyright 2016 ZXing authors
// Copyright 2017-2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <span>
#include <vector>

namespace librscpp {

/**
 * @brief Represents a polynomial whose coefficients are elements of Field.
 * 
 * The coefficients are stored in a vector, arranged from most significant (highest-power term) to least significant.
 */
template <typename Field>
class Poly : public std::vector<typename Field::value_type>
{
	using T = typename Field::value_type;
	using Base = std::vector<T>;

public:
	void trimLeft(ptrdiff_t delta) { Base::erase(begin(), begin() + delta); }

	void normalize(int start = 0)
	{
		auto firstNonZero = std::find_if(begin() + std::min<int>(start, size()), end(), [](T c) { return c != 0; });
		trimLeft(firstNonZero - begin());
	}

	using Base::at;
	using Base::capacity;
	using Base::front;

public:
	using Base::begin;
	using Base::end;
	using Base::reserve;
	using Base::size;

	const Field& field;

	Poly(const Field& field, size_t capacity = 0) : field(field) { reserve(capacity); }

	Poly(const Field& field, std::vector<T>&& coefficients, size_t capacity = 0) : Base(std::move(coefficients)), field(field)
	{
		reserve(capacity);
	}

	Poly(const Field& field, std::span<const T> coefficients, size_t capacity = 0) : field(field)
	{
		reserve(std::max(capacity, coefficients.size() + 1));
		Base::insert(end(), coefficients.begin(), coefficients.end());
	}

	Poly& operator=(Poly&& other) noexcept
	{
		assert(&field == &other.field);
		Base::operator=(std::move(other));
		return *this;
	};
	Poly(Poly&& other) noexcept = default;

	void resize(size_t size)
	{
		assert(size <= capacity());
		Base::resize(size);
	}

	Poly copy() const { return Poly(field, *this, capacity()); }
	T coef(int degree) const { return at(size() - 1 - degree); }
	T& coef(int degree) { return at(size() - 1 - degree); }
	int deg() const { return static_cast<int>(size()) - 1; }
	bool isZero() const { return size() == 0 || front() == 0; }

	/// @brief set to the monomial representing coefficient * x ^ degree
	void set(T coefficient, int degree = 0)
	{
		assert(degree >= 0);

		if (coefficient == 0 && degree == 0) {
			resize(0);
		} else {
			resize(degree + 1);
			std::ranges::fill(*this, 0);
			front() = coefficient;
		}
	}

	void sub(const Poly& rhs)
	{
		assert(&field == &rhs.field);

		auto oldCoef = [oldDeg = deg(), this](int i) { return i <= oldDeg ? at(oldDeg - i) : 0; };
		resize(std::max(size(), rhs.size()));
		for (int i = 0; i <= rhs.deg(); ++i)
			coef(i) = field.sub(oldCoef(i), rhs.coef(i));

		normalize();
	}

	void mul(const Poly& rhs, int trimDeg = 0)
	{
		assert(this != &rhs); // self-multiplication is not supported
		assert(&field == &rhs.field);

		int lhsSize = size();
		int rhsSize = rhs.size();
		int productSize = lhsSize + rhsSize - 1;

		if (trimDeg == 0) {
			// inplace multiplication
			resize(productSize);
			for (int p = productSize - 1; p >= 0; --p) {
				T acc = 0;
				for (int i = std::max(0, p - (rhsSize - 1)); i <= std::min(lhsSize - 1, p); ++i)
					acc = field.add(acc, field.mul(at(i), rhs.at(p - i)));
				at(p) = acc;
			}
		} else {
			// canonical long multiplication with optional left or right side trimming
			std::vector<T> res(productSize, 0);
			for (int i = 0; i < lhsSize; ++i)
				for (int j = 0; j < rhsSize; ++j)
					res[i + j] = field.add(res[i + j], field.mul(at(i), rhs.at(j)));
			resize(std::abs(trimDeg));
			if (trimDeg < 0)
				std::move(res.end() + trimDeg, res.end(), begin());
			else
				std::move(res.begin(), res.begin() + trimDeg, begin());
		}
	}

	void mul(T coefficient, int degree = 0)
	{
		assert(degree >= 0);

		if (coefficient == 0) {
			set(0);
		} else {
			for (T& c : *this)
				c = field.mul(c, coefficient);
			resize(size() + degree);
		}
	}

	void div(const Poly& divisor, Poly& quotient)
	{
		assert(&field == &divisor.field && &field == &quotient.field);
		assert(!divisor.isZero());

		// Use Expanded Synthetic Division (see https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders)
		// and keep quotient/remainder storage separate to avoid cross-contaminating capacity roles.
		quotient.resize(deg() < divisor.deg() ? 0 : size() - divisor.deg());
		auto normalizer = field.inv(divisor.at(0));
		for (size_t i = 0; i < quotient.size(); ++i) {
			auto& ci = at(i);
			if (ci != 0) {
				ci = field.mul(ci, normalizer);

				// we always skip the first coefficient of the divisor, because it's only used to normalize the dividend coefficient
				for (size_t j = 1; j < divisor.size(); ++j)
					at(i + j) = field.sub(at(i + j), field.mul(divisor.at(j), ci)); // equivalent to: result[i + j] -= divisor[j] * ci
			}
			quotient.at(i) = ci;
		}

		// extract the normalized remainder from result
		normalize(quotient.size());
	}

	void neg()
	{
		for (T& c : *this)
			c = field.neg(c);
	}

	template <typename U = T>
	static T evaluate(const Field& field, std::span<const U> coeffs, T a)
	{
		return std::accumulate(coeffs.begin(), coeffs.end(), T(0),
							   [&field, a](T res, U coeff) { return field.add(field.mul(a, res), coeff); });
	}

	/// @brief evaluation of this polynomial at a given point
	T evaluate(T a) const { return evaluate(field, std::span(*this), a); }
};

} // namespace librscpp
