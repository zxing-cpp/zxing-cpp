#pragma once
/*
* Copyright 2017 Axel Waggershauser
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

#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <cstring>

namespace ZXing {

template <typename Container, typename Value>
auto Find(const Container& c, const Value& v) -> decltype(std::begin(c)) {
    return std::find(std::begin(c), std::end(c), v);
}

template <typename Container, typename Predicate>
auto FindIf(Container& c, Predicate p) -> decltype(std::begin(c)) {
    return std::find_if(std::begin(c), std::end(c), p);
}

template <typename Container, typename Value>
auto Contains(const Container& c, const Value& v) -> decltype(std::begin(c), bool()){
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

template <typename ListType, typename Value>
auto Contains(const std::initializer_list<ListType>& c, const Value& v) -> decltype(std::begin(c), bool()){
    return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

inline bool Contains(const char* str, char c) {
	return strchr(str, c) != nullptr;
}

template <typename Container, typename Value>
Value Accumulate(const Container& c, Value v = Value(0)) {
    return std::accumulate(std::begin(c), std::end(c), v);
}

template <typename T, typename S = int>
constexpr S Length(const T&) {
	return static_cast<S>(std::extent<T>::value);
}

template <typename Container, typename Value>
int IndexOf(const Container& c, const Value& v) {
	auto i = Find(c, v);
	return i == std::end(c) ? -1 : static_cast<int>(i - std::begin(c));
}

inline int IndexOf(const char* str, char c) {
	auto s = strchr(str, c);
	return s != nullptr ? static_cast<int>(s - str) : -1;
}

template <typename Container, typename Value, class UnaryOp>
Value TransformReduce(const Container& c, Value s, UnaryOp op) {
	for (const auto& v : c)
		s += op(v);
	return s;
}

} // ZXing
