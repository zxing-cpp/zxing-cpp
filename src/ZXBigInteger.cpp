/*
* Copyright 2016 Huy Cuong Nguyen
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

#include "ZXBigInteger.h"
#include <algorithm>

namespace ZXing {

typedef std::vector<BigInteger::Block> Magnetude;

static void AddMag(const Magnetude& a, const Magnetude& b, Magnetude& c)
{
	Magnetude tmp;
	Magnetude& r = &c == &a || &c == &b ? tmp : c;

	// a2 points to the longer input, b2 points to the shorter
	const Magnetude& a2 = a.size() >= b.size() ? a : b;
	const Magnetude& b2 = a.size() >= b.size() ? b : a;
	r.resize(a2.size() + 1);
	size_t i = 0;
	bool carryIn = false;
	for (; i < b2.size(); ++i) {
		auto temp = a2[i] + b2[i];
		bool carryOut = (temp < a2[i]);
		if (carryIn) {
			++temp;
			carryOut |= (temp == 0);
		}
		r[i] = temp;
		carryIn = carryOut;
	}
	// If there is a carry left over, increase blocks until one does not roll over.
	for (; i < a2.size() && carryIn; ++i) {
		auto temp = a2[i] + 1;
		carryIn = (temp == 0);
		r[i] = temp;
	}
	// If the carry was resolved but the larger number still has blocks, copy them over.
	for (; i < a2.size(); ++i) {
		r[i] = a2[i];
	}
	// Set the extra block if there's still a carry, decrease length otherwise
	if (carryIn) {
		r[i] = 1;
	}
	else {
		r.resize(r.size() - 1);
	}

	c = r;
}

// Note that we DO NOT support the case where b is greater than a.
static void SubMag(const Magnetude& a, const Magnetude& b, Magnetude& c)
{
	if (b.empty()) {
		c = a;
		return;
	}

	Magnetude tmp;
	Magnetude& r = &c == &a || &c == &b ? tmp : c;

	//if (a.size() < b.size()) {
	// throw error;
	//}

	r.resize(a.size());
	size_t i = 0;
	bool borrowIn = false;
	for (; i < b.size(); ++i) {
		auto temp = a[i] - b[i];
		// If a reverse rollover occurred, the result is greater than the block from a.
		bool borrowOut = (temp > a[i]);
		if (borrowIn) {
			borrowOut |= (temp == 0);
			temp--;
		}
		r[i] = temp;
		borrowIn = borrowOut;
	}
	// If there is a borrow left over, decrease blocks until one does not reverse rollover.
	for (; i < a.size() && borrowIn; ++i) {
		borrowIn = (a[i] == 0);
		r[i] = a[i] - 1;
	}
	//if (borrowIn) {
	//throw error;
	//}
	// Copy over the rest of the blocks
	for (; i < a.size(); ++i) {
		r[i] = a[i];
	}

	// Zap leading zeros
	while (!r.empty() && r.back() == 0) {
		r.resize(r.size() - 1);
	}

	c = r;
}

typedef Magnetude::value_type Block;
static const size_t NB_BITS = 8 * sizeof(Block);

inline Block GetShiftedBlock(const Magnetude& num, size_t x, size_t y)
{
	Block part1 = (x == 0 || y == 0) ? Block(0) : (num[x - 1] >> (NB_BITS - y));
	Block part2 = (x == num.size()) ? Block(0) : (num[x] << y);
	return part1 | part2;
}

static void MulMag(const Magnetude& a, const Magnetude& b, Magnetude& c)
{
	// If either a or b is zero, set to zero.
	if (a.empty() || b.empty()) {
		c.clear();
		return;
	}

	Magnetude tmp;
	Magnetude& r = &c == &a || &c == &b ? tmp : c;

	/*
	* Overall method:
	*
	* Set this = 0.
	* For each 1-bit of `a' (say the `i2'th bit of block `i'):
	*    Add `b << (i blocks and i2 bits)' to *this.
	*/
	// Variables for the calculation
	//Index i, j, k;
	//unsigned int i2;
	//Blk temp;
	//bool carryIn, carryOut;
	// Set preliminary length and make room
	r.resize(a.size() + b.size());
	std::fill(r.begin(), r.end(), 0);

	// For each block of the first number...
	for (size_t i = 0; i < a.size(); ++i) {
		// For each 1-bit of that block...
		for (size_t i2 = 0; i2 < NB_BITS; ++i2) {
			if ((a[i] & (Block(1) << i2)) == 0)
				continue;
			/*
			* Add b to this, shifted left i blocks and i2 bits.
			* j is the index in b, and k = i + j is the index in this.
			*
			* `getShiftedBlock', a short inline function defined above,
			* is now used for the bit handling.  It replaces the more
			* complex `bHigh' code, in which each run of the loop dealt
			* immediately with the low bits and saved the high bits to
			* be picked up next time.  The last run of the loop used to
			* leave leftover high bits, which were handled separately.
			* Instead, this loop runs an additional time with j == b.len.
			* These changes were made on 2005.01.11.
			*/
			size_t k = i;
			bool carryIn = false;
			for (size_t j = 0; j <= b.size(); ++j, ++k) {
				/*
				* The body of this loop is very similar to the body of the first loop
				* in `add', except that this loop does a `+=' instead of a `+'.
				*/
				auto temp = r[k] + GetShiftedBlock(b, j, i2);
				auto carryOut = (temp < r[k]);
				if (carryIn) {
					temp++;
					carryOut |= (temp == 0);
				}
				r[k] = temp;
				carryIn = carryOut;
			}
			// No more extra iteration to deal with `bHigh'.
			// Roll-over a carry as necessary.
			for (; carryIn; k++) {
				r[k]++;
				carryIn = (r[k] == 0);
			}
		}
	}
	// Zap possible leading zero
	if (r.back() == 0) {
		r.resize(r.size() - 1);
	}

	c = r;
}

static int CompareMag(const Magnetude& a, const Magnetude& b)
{
	// A bigger length implies a bigger number.
	if (a.size() < b.size()) {
		return -1;
	}
	else if (a.size() > b.size()) {
		return 1;
	}
	else {
		// Compare blocks one by one from left to right.
		auto p = std::mismatch(a.rbegin(), a.rend(), b.rbegin());
		if (p.first != a.rend()) {
			return *p.first < *p.second ? -1 : 1;	// note: cannot use substraction here
		}
		return 0;
	}
}

void
BigInteger::Add(const BigInteger& a, const BigInteger &b, BigInteger& c)
{
	// If one argument is zero, copy the other.
	if (a.mag.empty()) {
		c = b;
		return;
	}
	if (b.mag.empty()) {
		c = a;
		return;
	}

	// If the arguments have the same sign, take the
	// common sign and add their magnitudes.
	if (a.negative == b.negative) {
		c.negative = a.negative;
		AddMag(a.mag, b.mag, c.mag);
	}
	else {
		// Otherwise, their magnitudes must be compared.
		int cmp = CompareMag(a.mag, b.mag);
		if (cmp < 0) {
			c.negative = b.negative;
			SubMag(b.mag, a.mag, c.mag);
		}
		else if (cmp > 0) {
			c.negative = a.negative;
			SubMag(a.mag, b.mag, c.mag);
		}
		else {
			c.negative = false;
			c.mag.clear();
		}
	}
}

void
BigInteger::Subtract(const BigInteger &a, const BigInteger &b, BigInteger& c)
{
	if (a.mag.empty()) {
		c.negative = !b.negative;
		c.mag = b.mag;
		return;
	}
	else if (b.mag.empty()) {
		c = a;
		return;
	}

	// If their signs differ, take a.sign and add the magnitudes.
	if (a.negative != b.negative) {
		c.negative = a.negative;
		AddMag(a.mag, b.mag, c.mag);
	}
	else {
		int cmp = CompareMag(a.mag, b.mag);
		if (cmp < 0) {
			c.negative = !b.negative;
			SubMag(b.mag, a.mag, c.mag);
		}
		else if (cmp > 0) {
			c.negative = a.negative;
			SubMag(a.mag, b.mag, c.mag);
		}
		else {
			c.negative = false;
			c.mag.clear();
		}
	}
}

void
BigInteger::Multiply(const BigInteger &a, const BigInteger &b, BigInteger& c)
{
	if (a.mag.empty() || b.mag.empty()) {
		c.negative = false;
		c.mag.clear();
		return;
	}
	c.negative = a.negative != b.negative;
	MulMag(a.mag, b.mag, c.mag);
}


} // ZXing