#pragma once
#include <random>

namespace ZXing {

class PseudoRandom {
	std::minstd_rand _random;

public:
	PseudoRandom(size_t seed) : _random(static_cast<std::minstd_rand::result_type>(seed)) {}

	template <typename IntType>
	IntType next(IntType low, IntType high) {
		return std::uniform_int_distribution<IntType>(low, high)(_random);
	}
};

}