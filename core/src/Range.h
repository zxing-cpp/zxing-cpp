/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ZXAlgorithms.h"

namespace ZXing {

template <typename Iterator>
struct StrideIter
{
	Iterator pos;
	int stride;

	auto operator*() const { return *pos; }
	auto operator[](int i) const { return *(pos + i * stride); }
	StrideIter<Iterator>& operator++() { return pos += stride, *this; }
	bool operator<(const StrideIter<Iterator>& rhs) const { return pos < rhs.pos; }
	StrideIter<Iterator> operator+(int i) const { return {pos + i * stride, stride}; }
	int operator-(const StrideIter<Iterator>& rhs) const { return (pos - rhs.pos) / stride; }
};

template <typename Iterator>
struct Range
{
	Iterator _begin, _end;

	Iterator begin() const noexcept { return _begin; }
	Iterator end() const noexcept { return _end; }
	explicit operator bool() const { return begin() < end(); }
	int size() const { return narrow_cast<int>(end() - begin()); }
};

} // namespace ZXing
