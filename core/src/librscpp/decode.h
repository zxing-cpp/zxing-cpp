// Copyright 2016 ZXing authors
// Copyright 2017-2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "field.h"
#include "poly.h"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace librscpp {

namespace {

template <typename Field>
std::optional<std::tuple<Poly<Field>, Poly<Field>>> sugiyama_algorithm(Poly<Field>&& syndromes)
{
	// See https://en.wikipedia.org/wiki/Reed–Solomon_error_correction#Sugiyama_decoder
	auto& field = syndromes.field;
	int R = syndromes.size();
	syndromes.normalize();
	size_t fullSize = R + 1;
	size_t halfSize = (R + 1) / 2 + 1; // ceil(R / 2) + 1
	Poly<Field> r(field, std::move(syndromes), fullSize), rLast(field, fullSize);
	Poly<Field> tLast(field, halfSize), t(field, halfSize), q(field, halfSize);

	rLast.set(1, R);
	tLast.set(0);
	t.set(1);

	// Run extended Euclidean algorithm until r's degree is less than R/2
	while (r.deg() >= R / 2) {
		swap(tLast, t);
		swap(rLast, r);

		// Divide r by rLast, with quotient in q and remainder in r
		r.div(rLast, q);
		assert(r.deg() < rLast.deg());

		q.mul(tLast);
		t.sub(q);
	}

	if (t.coef(0) == 0)
		return {};

	int inv_t0 = field.inv(t.coef(0));
	t.mul(inv_t0);
	r.mul(inv_t0);

	// locator/lamda is t, evaluator/omega is r
	return std::tuple{std::move(t), std::move(r)};
}

template <typename Field>
static std::vector<int> find_locations(const Poly<Field>& locator)
{
	// This is a brute force search for roots of locator (not Chien's search)
	std::vector<int> res;
	res.reserve(locator.deg());

	for (int i = 1; i < locator.field.size() && res.size() < res.capacity(); i++)
		if (locator.evaluate(i) == 0)
			res.push_back(locator.field.inv(i));

	return res;
}

template <typename Field>
static std::vector<int> find_magnitudes(const Poly<Field>& evaluator, const Poly<Field>& locator [[maybe_unused]],
										const std::vector<int>& locations)
{
	// This is directly applying Forney's Formula
	auto& field = evaluator.field;
	int numErrors = std::ssize(locations);
	std::vector<int> res(numErrors);
	for (int i = 0; i < numErrors; ++i) {
		int xiInverse = field.inv(locations[i]);
		int denom = 1;
		for (int j = 0; j < numErrors; ++j)
			if (i != j)
				denom = field.mul(denom, field.sub(1, field.mul(locations[j], xiInverse)));
		res[i] = field.mul(evaluator.evaluate(xiInverse), field.inv(denom));
		if (field.fcr() != 0)
			res[i] = field.mul(res[i], xiInverse);
	}
	return res;
}

#if 0
template <>
std::vector<int> find_magnitudes(const Poly<GFpPDF417>& evaluator, const Poly<GFpPDF417>& locator,
								 const std::vector<int>& locations)
{
	const auto& field = evaluator.field;

	Poly<GFpPDF417> formalDerivative(field, locator.size());
	formalDerivative.resize(locator.deg());
	for (int i = 1; i <= locator.deg(); i++)
		formalDerivative.coef(i - 1) = field.mul(i, locator.coef(i));

	// This is directly applying Forney's Formula
	std::vector<int> result(locations.size());
	for (size_t i = 0; i < result.size(); i++) {
		int xiInverse = field.inv(locations[i]);
		int numerator = field.sub(0, evaluator.evaluate(xiInverse));
		int denominator = field.inv(formalDerivative.evaluate(xiInverse));
		result[i] = field.mul(numerator, denominator);
	}
	return result;
}
#endif

// Compute the Reed-Solomon syndrome polynomial for the given codeword.
// The returned polynomial has degree < numECC and is zero iff the codeword is valid.
template <typename Field, typename T>
Poly<Field> compute_syndromes(const Field& field, std::span<const T> codeword, int numECC)
{
#if 0
	Poly<Field> syndromes(field, numECC + 1);
	for (int i = 0; i < numECC; i++)
		syndromes.push_back(Poly<Field>::evaluate(field, codeword, field.exp(numECC - 1 - i + field.fcr())));
	return syndromes;
#else
	// The following cache friendlier version is 2x to 5x faster than the straightforward one above
	std::vector<typename Field::value_type> roots(numECC);
	for (int i = 0; i < numECC; ++i)
		roots[i] = field.exp(numECC - 1 - i + field.fcr());

	std::vector<typename Field::value_type> res(numECC + 1, 0);
	for (auto coeff : codeword)
		for (int i = 0; i < numECC; ++i)
			res[i] = field.add(field.mul(roots[i], res[i]), static_cast<std::make_unsigned_t<T>>(coeff));
	res.resize(numECC);

	return Poly<Field>(field, std::move(res));
#endif
}

}

/**
 * Decode and correct a Reed-Solomon codeword in place.
 *
 * @param field Finite field instance used by the RS code.
 * @param codeword Input/output codeword symbols; corrected in place on success.
 * @param numECC Number of ECC/parity symbols in the codeword.
 * @param erasures Optional symbol indices known to be erased.
 * @return Number of parity symbols consumed for correction (2*errors + erasures),
 *         0 if no correction was needed, or std::nullopt if decoding failed.
 * @throws std::invalid_argument If erasure count exceeds numECC or an erasure index is out of range.
 */
template <typename Field, std::integral T>
[[nodiscard]] std::optional<int> decode(const Field& field, std::span<T> codeword, int numECC, std::span<const int> erasures = {})
{
	// See https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction and
	// https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders for details on how RS works. See
	// https://mthsc.clemson.edu/misc/MAM_2014/bmj9b.pdf for details on how to handle erasures and potential optimizations.

	int cwLen = std::ssize(codeword);
	int numErasures = std::ssize(erasures);

	if (numErasures > numECC)
		throw std::invalid_argument("Too many erasures to correct");
	if (std::ranges::any_of(erasures, [cwLen](int e) { return e < 0 || e >= cwLen; }))
		throw std::invalid_argument("Erasure position out of range");

	if constexpr (std::is_same_v<Field, GFp<typename Field::value_type>>) {
		// prevent potentially out-of-range values from causing out-of-bounds access in GFp's exp/log tables
		for (auto& symbol : codeword)
			symbol = std::clamp(symbol, 0, field.size() - 1);
	}

	auto syndromes = compute_syndromes<Field, T>(field, codeword, numECC);

	if (std::ranges::all_of(syndromes, [](auto c) { return c == 0; }))
		return 0;

	Poly<Field> oriSyndromes(field), erasureLocator(field, numErasures + 1);

	// If there are erasures, we modify the syndromes to "remove" the effect of those...
	if (!erasures.empty()) {
		// Keep a copy of the original syndromes for later use in Forney's formula
		oriSyndromes = syndromes.copy();

		// Erasure locator: Λe(x) = ∏ (1 - Xi x), Xi = α^(cwLen - 1 - pos)
		erasureLocator.set(1);
		Poly<Field> term(field, {0, 1});
		for (int pos : erasures) {
			term.coef(1) = field.neg(field.exp((cwLen - 1 - pos)));
			erasureLocator.mul(term);
		}

		// S'(x) = Λe(x) * S(x)
		syndromes.mul(erasureLocator, numECC); // keep only the hightest numECC coefficients
		syndromes.trimLeft(numErasures); // remove the highest numErasures coefficients, equivalent to dividing by x^(numECC - numErasures)
		// syndromes now only contains the "error syndromes"
	}

	auto result = sugiyama_algorithm(std::move(syndromes));
	if (!result)
		return {};

	auto [locator, evaluator] = std::move(*result); // Λ is error locator, Ω is error evaluator

	int numErrors = locator.deg();
	if (2 * numErrors + numErasures > numECC)
		return {};

	if (!erasures.empty()) {
		// Λ'(x) = Λ(x) * Λe(x), i.e. merge erasure locator into error locator
		locator.reserve(locator.size() + erasureLocator.size());
		locator.mul(erasureLocator);

		// Recompute evaluator/Ω so it matches the final combined locator/Λ'.
		evaluator = std::move(oriSyndromes);
		evaluator.mul(locator, -numECC); // keep only the lowest numECC coefficients
	}

	auto locations = find_locations(locator);
	if (std::ssize(locations) != numErrors + numErasures)
		return {}; // Error locator degree does not match number of roots, most likely there are more errors than can be recovered

	auto magnitudes = find_magnitudes(evaluator, locator, locations);

	for (int i = 0; i < numErrors + numErasures; ++i) {
		int pos = cwLen - 1 - field.log(locations[i]);
		if (pos < 0)
			return {};
		codeword[pos] = field.sub(codeword[pos], magnitudes[i]);
	}

#if 1
	// re-evaluate the syndromes of the recovered codeword to make sure it is a valid codeword now (see #940-3)
	syndromes = compute_syndromes<Field, T>(field, codeword, numECC);
	if (std::ranges::any_of(syndromes, [](auto c) { return c != 0; }))
		return {};
#endif

	// if (numErasures)
	// 	printf("erasures: %d, errors: %d, ecc: %d\n", numErasures, numErrors, numECC);

	// Correcting 1 error consumes 2 parity symbols, correcting 1 erasure consumes 1 parity symbol.
	return 2 * numErrors + numErasures;
}

template <typename Field, std::ranges::contiguous_range R>
	requires std::integral<std::ranges::range_value_t<R>>
[[nodiscard]] std::optional<int> decode(const Field& field, R&& codeword, int numECC, std::span<const int> erasures = {})
{
	using T = std::ranges::range_value_t<R>;
	return decode<Field, T>(field, std::span(codeword), numECC, erasures);
}

} // namespace librscpp
