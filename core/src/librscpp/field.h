// Copyright 2016 ZXing authors
// Copyright 2017-2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

// #define LIBRSCPP_SAVE_MEMORY

namespace librscpp {

template <std::unsigned_integral T, std::integral HP>
class GFBase
{
	static_assert(sizeof(T) <= sizeof(HP) && sizeof(HP) >= 2,
				  "HP must be able to hold values of type T and the result of their addition");

protected:
	const int _size;
	const int _fcr;
	std::vector<T> _expTable, _logTable;

	// avoid using the '%' modulo operator => decode computation can be more than twice as fast (depending on architecture)
	// see also https://stackoverflow.com/a/33333636/2088798
	inline static T fast_mod(HP input, HP mod) { return input < mod ? input : input - mod; }

	GFBase(int size, int fcr, std::function<T(T)> next) : _size(size), _fcr(fcr)
	{
		if (size <= 0 || size > (1 << (sizeof(T) * 8)))
			throw std::invalid_argument("Invalid field size " + std::to_string(size));
		if (fcr < 0 || fcr >= size)
			throw std::invalid_argument("Invalid first consecutive root " + std::to_string(fcr) + " for field of size "
										+ std::to_string(size));

#ifdef LIBRSCPP_SAVE_MEMORY
		_expTable.resize(size, 0);
#else
		_expTable.resize(size * 2, 0);
#endif
		_logTable.resize(size, 0);
		int x = 1;
		for (int i = 0; i < size; ++i) {
			_expTable[i] = x;
			x = next(x);
		}

#ifndef LIBRSCPP_SAVE_MEMORY
		for (int i = size - 1; i < size * 2; ++i)
			_expTable[i] = _expTable[i - (size - 1)];
#endif

		for (int i = 0; i < size - 1; ++i)
			_logTable[_expTable[i]] = i;
		// logTable[0] == 0 but this should never be used
	}

public:
	using value_type = T;

	// 2 to the power of a
	T exp(T a) const { return _expTable.at((a)); }

	// base 2 log of a
	T log(T a) const
	{
		assert(a != 0);
		return _logTable.at((a));
	}

	// multiplicative inverse of a (== 1/a)
	T inv(T a) const { return _expTable[_size - 1 - log(a)]; }

	// product of a and b
	T mul(T a, T b) const noexcept
	{
		if (a == 0 || b == 0)
			return 0;

		// this is in the hot path, operator[] is around 20% faster than at() for the unit tests
#ifdef LIBRSCPP_SAVE_MEMORY
		return _expTable[fast_mod(HP(_logTable[a]) + HP(_logTable[b]), _size - 1)];
#else
		return _expTable[HP(_logTable[a]) + HP(_logTable[b])];
#endif
	}

	int size() const noexcept { return _size; }

	// first consecutive root of the generator polynomial, sometimes also called "b" or "base"
	int fcr() const noexcept { return _fcr; }
};

/**
 * @brief Represents an extension Galois Field GF(2^n).
 */
template <std::unsigned_integral T = uint16_t, std::integral HP = uint32_t>
class GF2n : public GFBase<T, HP>
{
public:
	/**
	 * @brief Create a Galois Field GF(2^n) using the given primitive polynomial.
	 *
	 * @param n the size of the field is 2^n, n is called the "word size".
	 * @param primitive irreducible polynomial whose coefficients are represented by
	 *  the bits of an int, where the least-significant bit represents the constant
	 *  coefficient
	 * @param fcr the first consecutive root of the generator polynomial, sometimes also called "b" or "base"
	 *  (g(x) = (x+a^b)(x+a^(b+1))...(x+a^(b+2t-1))).
	 *  This is typically 1, but for e.g. QR Code it is 0.
	 */
	GF2n(uint32_t n, uint32_t primitive, int fcr)
		: GFBase<T, HP>(1 << n, fcr, [primitive, n](T x) {
			  HP size = 1 << n;
			  HP ix = x;
			  ix *= 2;
			  if (ix >= size) {
				  ix ^= primitive;
				  ix &= size - 1;
			  }
			  return ix;
		  })
	{
		if (primitive < (1 << n) || primitive >= (1 << (n + 1)) || (primitive & 1) == 0)
			throw std::invalid_argument("Invalid primitive polynomial " + std::to_string(primitive) + " for GF(2^" + std::to_string(n)
										+ ')');
	}

	T add(T a, T b) const { return a ^ b; }
	T sub(T a, T b) const { return a ^ b; } // a - b == a + b in GF(2^n)
	T neg(T a) const { return a; }          // -a == a in GF(2^n)
};

/**
 * @brief Represents a prime Galois Field GF(p).
 */
template <std::unsigned_integral T = uint16_t, std::integral HP = uint32_t>
class GFp : public GFBase<T, HP>
{
	using Base = GFBase<T, HP>;

public:
	/**
	 * @brief Create a Galois Field GF(p) using the given prime.
	 *
	 * @param prime a prime number representing the size of the field
	 * @param primitive a primitive root modulo prime, representing the generator of the field
	 * @param fcr the first consecutive root of the generator polynomial, sometimes also called "b" or "base"
	 */
	GFp(uint32_t prime, uint32_t primitive, int fcr)
		: GFBase<T, HP>(prime, fcr, [prime, primitive](T x) { return (x * primitive) % prime; })
	{}

	T add(T a, T b) const { return Base::fast_mod(HP(a) + HP(b), Base::size()); }
	T sub(T a, T b) const { return Base::fast_mod(Base::size() + HP(a) - HP(b), Base::size()); }
	T neg(T a) const { return sub(0, a); }
};

} // namespace librscpp
