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
#include "BitHacks.h"
#include <algorithm>

namespace ZXing {

typedef BigInteger::Block Block;
typedef std::vector<Block> Magnetude;

static const size_t NB_BITS = 8 * sizeof(Block);

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

static inline Block GetShiftedBlock(const Magnetude& num, size_t x, size_t y)
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

	r.clear();
	r.resize(a.size() + b.size(), 0);

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

/*
* DIVISION WITH REMAINDER
* This monstrous function mods *this by the given divisor b while storing the
* quotient in the given object q; at the end, *this contains the remainder.
* The seemingly bizarre pattern of inputs and outputs was chosen so that the
* function copies as little as possible (since it is implemented by repeated
* subtraction of multiples of b from *this).
*
* "modWithQuotient" might be a better name for this function, but I would
* rather not change the name now.
*/
void DivideWithRemainder(const Magnetude& a, const Magnetude& b, Magnetude& qq, Magnetude& rr)
{
	/* Defending against aliased calls is more complex than usual because we
	* are writing to both r and q.
	*
	* It would be silly to try to write quotient and remainder to the
	* same variable.  Rule that out right away. */
	//if (&rr == &qq) {
	//	throw std::invalid_argument("Quotient and remainder should be the same destination");
	//}

	Magnetude tmp, tmp2;
	Magnetude& q = &qq == &a || &qq == &b ? tmp : qq;
	Magnetude& r = &rr == &b ? tmp2 : rr;


	/*
	* Knuth's definition of mod (which this function uses) is somewhat
	* different from the C++ definition of % in case of division by 0.
	*
	* We let a / 0 == 0 (it doesn't matter much) and a % 0 == a, no
	* exceptions thrown.  This allows us to preserve both Knuth's demand
	* that a mod 0 == a and the useful property that
	* (a / b) * b + (a % b) == a.
	*/
	/*
	* If a.len < b.len, then a < b, and we can be sure that b doesn't go into
	* a at all.  The quotient is 0 and *this is already the remainder (so leave it alone).
	*/
	if (b.empty() || a.size() < b.size()) {
		q.clear();
		r = a;
		return;
	}

	// At this point we know a.len >= b.len > 0.  (Whew!)

	/*
	* Overall method:
	*
	* For each appropriate i and i2, decreasing:
	*    Subtract (b << (i blocks and i2 bits)) from *this, storing the
	*      result in subtractBuf.
	*    If the subtraction succeeds with a nonnegative result:
	*        Turn on bit i2 of block i of the quotient q.
	*        Copy subtractBuf back into *this.
	*    Otherwise bit i2 of block i remains off, and *this is unchanged.
	*
	* Eventually q will contain the entire quotient, and *this will
	* be left with the remainder.
	*
	* subtractBuf[x] corresponds to blk[x], not blk[x+i], since 2005.01.11.
	* But on a single iteration, we don't touch the i lowest blocks of blk
	* (and don't use those of subtractBuf) because these blocks are
	* unaffected by the subtraction: we are subtracting
	* (b << (i blocks and i2 bits)), which ends in at least `i' zero
	* blocks. */

	/*
	* Make sure we have an extra zero block just past the value.
	*
	* When we attempt a subtraction, we might shift `b' so
	* its first block begins a few bits left of the dividend,
	* and then we'll try to compare these extra bits with
	* a nonexistent block to the left of the dividend.  The
	* extra zero block ensures sensible behavior; we need
	* an extra block in `subtractBuf' for exactly the same reason.
	*/
	r.resize(a.size() + 1);
	if (&r != &a) {
		std::copy(a.begin(), a.end(), r.begin());
	}
	r.back() = 0;

	Magnetude subtractBuf(r.size());

	// Set preliminary length for quotient and make room
	q.resize(a.size() - b.size() + 1);

	// For each possible left-shift of b in blocks...
	size_t i = q.size();
	while (i > 0) {
		i--;
		// For each possible left-shift of b in bits...
		// (Remember, N is the number of bits in a Blk.)
		q[i] = 0;
		size_t i2 = NB_BITS;
		while (i2 > 0) {
			i2--;
			/*
			* Subtract b, shifted left i blocks and i2 bits, from *this,
			* and store the answer in subtractBuf.  In the for loop, `k == i + j'.
			*
			* Compare this to the middle section of `multiply'.  They
			* are in many ways analogous.  See especially the discussion
			* of `getShiftedBlock'.
			*/
			size_t k = i;
			bool borrowIn = false;
			for (size_t j = 0; j <= b.size(); ++j, ++k) {
				auto temp = r[k] - GetShiftedBlock(b, j, i2);
				bool borrowOut = (temp > r[k]);
				if (borrowIn) {
					borrowOut |= (temp == 0);
					temp--;
				}
				// Since 2005.01.11, indices of `subtractBuf' directly match those of `blk', so use `k'.
				subtractBuf[k] = temp;
				borrowIn = borrowOut;
			}
			// No more extra iteration to deal with `bHigh'.
			// Roll-over a borrow as necessary.
			for (; k < a.size() && borrowIn; k++) {
				borrowIn = (r[k] == 0);
				subtractBuf[k] = r[k] - 1;
			}
			/*
			* If the subtraction was performed successfully (!borrowIn),
			* set bit i2 in block i of the quotient.
			*
			* Then, copy the portion of subtractBuf filled by the subtraction
			* back to *this.  This portion starts with block i and ends--
			* where?  Not necessarily at block `i + b.len'!  Well, we
			* increased k every time we saved a block into subtractBuf, so
			* the region of subtractBuf we copy is just [i, k).
			*/
			if (!borrowIn) {
				q[i] |= (Block(1) << i2);
				while (k > i) {
					k--;
					r[k] = subtractBuf[k];
				}
			}
		}
	}
	// Zap possible leading zero in quotient
	if (q.back() == 0)
		q.resize(q.size() - 1);
	
	// Zap any/all leading zeros in remainder
	while (!r.empty() && r.back() == 0) {
		r.resize(r.size() - 1);
	}

	qq = q;
	rr = r;
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
	if (b.mag.empty()) {
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

size_t ceilingDiv(size_t a, size_t b) {
	return (a + b - 1) / b;
}

std::string
BigInteger::toString() const
{
	if (mag.empty()) {
		return "0";
	}
	std::string result;
	if (negative) {
		result.push_back('-');
	}

	static const uint32_t base = 10;
	auto maxBitLenOfX = static_cast<uint32_t>(mag.size()) * NB_BITS;
	int minBitsPerDigit = BitHacks::HighestBitSet(base);
	auto maxDigitLenOfX = (maxBitLenOfX + minBitsPerDigit - 1) / minBitsPerDigit; // ceilingDiv
	
	std::vector<uint8_t> buffer;
	buffer.reserve(maxDigitLenOfX);

	Magnetude x2 = mag;
	Magnetude buBase(1, base);
	Magnetude lastDigit;
	lastDigit.reserve(1);

	while (!x2.empty()) {
		// Get last digit.  This is like `lastDigit = x2 % buBase, x2 /= buBase'.
		DivideWithRemainder(x2, buBase, x2, lastDigit);
		// Save the digit.
		buffer.push_back(static_cast<uint8_t>(lastDigit.empty() ? 0 : lastDigit.front()));
	}

	size_t offset = result.size();
	result.resize(offset + buffer.size());
	std::transform(buffer.rbegin(), buffer.rend(), result.begin() + offset, [](uint8_t c) { return static_cast<char>('0' + c); });
	return result;
}

} // ZXing