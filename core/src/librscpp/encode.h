// Copyright 2016 ZXing authors
// Copyright 2017-2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "field.h"
#include "poly.h"

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace librscpp {

namespace {

template <typename Field>
Poly<Field> build_generator(const Field& field, int degree)
{
	Poly<Field> generator(field, degree + 1);
	generator.set(1);
	Poly<Field> term(field, {1, 0});
	for (int d = 1; d <= degree; d++) {
		term.coef(0) = field.neg(field.exp(d - 1 + field.fcr()));
		generator.mul(term);
	}
	return generator;
}

}

template <typename Field, std::integral T>
void encode(const Field& field, std::span<const T> message, std::span<T> ecc)
{
	if (ecc.empty())
		throw std::invalid_argument("Invalid number of error correction code words");

	using FT = typename Field::value_type;
	std::vector<FT> tmp;
	std::span<FT const> msg;
	if constexpr (!std::is_same_v<T, FT>) {
		// If the message type is different from the field's value type, we need to copy it into a temporary buffer of the right type.
		tmp.resize(message.size());
		std::ranges::transform(message, tmp.begin(), [](T c) { return static_cast<FT>(static_cast<std::make_unsigned_t<T>>(c)); });
		msg = tmp;
	} else {
		msg = std::span(message);
	}

	auto generator = build_generator(field, ecc.size());

	Poly<Field> info(field, msg, msg.size() + ecc.size());
	Poly<Field> _(field, msg.size() + 1);
	info.resize(msg.size() + ecc.size()); // left-shift == multiply by x^(ecc.size())
	info.div(generator, _);
	info.neg();
	auto out = std::fill_n(ecc.begin(), ecc.size() - info.size(), 0);
	std::ranges::transform(info, out, [](auto c) { return static_cast<T>(c); });
}

template <typename Field, std::ranges::contiguous_range R, std::ranges::contiguous_range S>
	requires std::ranges::sized_range<R> && std::ranges::sized_range<S> && std::same_as<std::ranges::range_value_t<R>, std::ranges::range_value_t<S>>
void encode(const Field& field, R&& message, S&& ecc)
{
	using T = std::ranges::range_value_t<R>;
	encode<Field, T>(field, std::span<const T>(message), std::span<T>(ecc));
}

template <typename Field, std::ranges::contiguous_range R>
	requires std::ranges::sized_range<R>
void encode(const Field& field, R&& message, int num_ecc)
{
	auto msg = std::span(message);
	encode(field, msg.subspan(0, msg.size() - num_ecc), msg.subspan(msg.size() - num_ecc, num_ecc));
}

} // namespace librscpp
