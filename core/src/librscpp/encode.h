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

/**
 * @brief Reed-Solomon systematic encoder.
 *
 * Computes parity symbols for @p data in the given Galois field and writes them to @p parity.
 *
 * @param field Galois field instance used for generator construction and arithmetic.
 * @param data Data symbols.
 * @param parity Output buffer for parity symbols. It's size determines the number of parity symbols.
 */
template <typename Field, std::integral T>
void encode(const Field& field, std::span<const T> data, std::span<T> parity)
{
	if (data.empty() || parity.empty())
		throw std::invalid_argument("Invalid number of data or parity symbols");

	using FT = typename Field::value_type;
	std::vector<FT> tmp;
	std::span<FT const> data_;
	if constexpr (!std::is_same_v<T, FT>) {
		// If the message type is different from the field's value type, we need to copy it into a temporary buffer of the right type.
		tmp.resize(data.size());
		std::ranges::transform(data, tmp.begin(), [](T sym) { return static_cast<FT>(static_cast<std::make_unsigned_t<T>>(sym)); });
		data_ = tmp;
	} else {
		data_ = std::span(data);
	}

	// See https://en.wikipedia.org/wiki/Reed–Solomon_error_correction#Systematic_encoding_procedure

	auto generator = build_generator(field, parity.size());

	Poly<Field> C(field, data_, data_.size() + parity.size());
	Poly<Field> _(field, data_.size() + 1);
	C.resize(data_.size() + parity.size()); // left-shift == multiply by x^(parity.size())
	C.div(generator, _);
	C.neg();
	auto out = std::fill_n(parity.begin(), parity.size() - C.size(), 0);
	std::ranges::transform(C, out, [](FT symbol) { return static_cast<T>(symbol); });
}

template <typename Field, std::ranges::contiguous_range R, std::ranges::contiguous_range S>
	requires std::same_as<std::ranges::range_value_t<R>, std::ranges::range_value_t<S>>
void encode(const Field& field, R&& data, S&& parity)
{
	using T = std::ranges::range_value_t<R>;
	encode<Field, T>(field, std::span<const T>(data), std::span<T>(parity));
}

/**
 * @brief Reed-Solomon in-place systematic encoder.
 *
 * Computes parity symbols for the data prefix in @p codeword and stores them
 * in the last @p num_parity elements of the same buffer.
 *
 * @param field Galois field instance used for generator construction and arithmetic.
 * @param codeword Buffer containing data symbols followed by parity storage.
 * @param num_parity Number of parity symbols to compute and write at the end of @p codeword.
 */
template <typename Field, std::ranges::contiguous_range R>
void encode_inplace(const Field& field, R&& codeword, int num_parity)
{
	std::span cw(codeword);
	auto data_size = std::ranges::ssize(codeword) - num_parity;
	if (data_size <= 0 || num_parity <= 0)
		throw std::invalid_argument("Invalid number of data or parity symbols");
	encode(field, cw.subspan(0, data_size), cw.subspan(data_size));
}

/**
 * @brief Reed-Solomon systematic encoder returning a full codeword.
 *
 * Appends @p num_parity parity symbols to @p data and returns the combined
 * codeword in a container of type @p Result.
 *
 * @param field Galois field instance used for generator construction and arithmetic.
 * @param data Input data symbols.
 * @param num_parity Number of parity symbols to append.
 * @return Result Codeword consisting of input data followed by parity symbols.
 */
template <typename Out = void, typename Field, std::ranges::contiguous_range In>
[[nodiscard]]
auto encode(const Field& field, In&& data, int num_parity)
{
	using T = std::conditional_t<std::is_void_v<Out>, std::remove_cvref_t<In>, Out>;
	T res;
	res.reserve(std::ranges::size(data) + num_parity);
	res.insert(res.end(), std::ranges::begin(data), std::ranges::end(data));
	res.resize(res.size() + num_parity);
	encode_inplace(field, res, num_parity);
	return res;
}

} // namespace librscpp
