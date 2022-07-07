/*
* Copyright 2017 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <string>

namespace ZXing {

template <class T, class U>
constexpr T narrow_cast(U&& u) noexcept {
	return static_cast<T>(std::forward<U>(u));
}

template <typename Container, typename Value>
auto Find(Container& c, const Value& v) -> decltype(std::begin(c)) {
	return std::find(std::begin(c), std::end(c), v);
}

template <typename Container, typename Predicate>
auto FindIf(Container& c, Predicate p) -> decltype(std::begin(c)) {
	return std::find_if(std::begin(c), std::end(c), p);
}

template <typename Container, typename Value>
auto Contains(const Container& c, const Value& v) -> decltype(std::begin(c), bool()){
	return Find(c, v) != std::end(c);
}

template <typename ListType, typename Value>
auto Contains(const std::initializer_list<ListType>& c, const Value& v) -> decltype(std::begin(c), bool()){
	return Find(c, v) != std::end(c);
}

inline bool Contains(const char* str, char c) {
	return strchr(str, c) != nullptr;
}

template <typename Container, typename Value = typename Container::value_type, typename Op = std::plus<Value>>
Value Reduce(const Container& c, Value v = Value{}, Op op = {}) {
	return std::accumulate(std::begin(c), std::end(c), v, op);
}

// see C++20 ssize
template <class Container>
constexpr auto Size(const Container& c) -> decltype(c.size(), int()) {
	return narrow_cast<int>(c.size());
}

template <class T, std::size_t N>
constexpr int Size(const T (&)[N]) noexcept {
	return narrow_cast<int>(N);
}

template <typename Container, typename Value>
int IndexOf(const Container& c, const Value& v) {
	auto i = Find(c, v);
	return i == std::end(c) ? -1 : narrow_cast<int>(std::distance(std::begin(c), i));
}

inline int IndexOf(const char* str, char c) {
	auto s = strchr(str, c);
	return s != nullptr ? narrow_cast<int>(s - str) : -1;
}

template <typename Container, typename Value, class UnaryOp>
Value TransformReduce(const Container& c, Value s, UnaryOp op) {
	for (const auto& v : c)
		s += op(v);
	return s;
}

} // ZXing
