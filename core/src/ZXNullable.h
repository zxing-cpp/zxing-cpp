#pragma once
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

#include <utility>
#include <stdexcept>

namespace ZXing {

template <typename T>
class Nullable final
{
	bool m_hasValue = false;
	T m_value;

public:
	Nullable() = default;
	Nullable(const T &value) : m_hasValue(true), m_value(value) {}
	Nullable(T &&value) : m_hasValue(true), m_value(std::move(value)) {}
	Nullable(std::nullptr_t) {}
	
	Nullable<T> & operator=(const T &value) {
		m_hasValue = true;
		m_value = value;
		return *this;
	}

	Nullable<T> & operator=(T &&value) {
		m_hasValue = true;
		m_value = std::move(value);
		return *this;
	}

	Nullable<T> & operator=(std::nullptr_t) {
		m_hasValue = false;
		m_value = T();
		return *this;
	}

	operator T() const {
		if (!m_hasValue) {
			throw std::logic_error("Access empty value");
		}
		return m_value;
	}
	
	bool hasValue() const {
		return m_hasValue;
	}
	
	const T & value() const {
		return m_value;
	}
	
	T & value() {
		return m_value;
	}

	friend inline bool operator==(const Nullable &a, const Nullable &b) {
		return a.m_hasValue == b.m_hasValue && (!a.m_hasValue || a.m_value == b.m_value);
	}
	friend inline bool operator!=(const Nullable &a, const Nullable &b) {
		return !(a == b);
	}

	friend inline bool operator==(const Nullable &a, const T &v) {
		return a.m_hasValue && a.m_value == v;
	}
	friend inline bool operator!=(const Nullable &a, const T &v) {
		return !(a == v);
	}

	friend inline bool operator==(const T &v, const Nullable &a) {
		return a.m_hasValue && a.m_value == v;
	}
	friend inline bool operator!=(const T &v, const Nullable &a) {
		return !(v == a);
	}

	friend inline bool operator==(const Nullable &a, std::nullptr_t) {
		return !a.m_hasValue;
	}
	friend inline bool operator!=(const Nullable &a, std::nullptr_t) {
		return a.m_hasValue;
	}

	friend inline bool operator==(std::nullptr_t, const Nullable &a) {
		return !a.m_hasValue;
	}
	friend inline bool operator!=(std::nullptr_t, const Nullable &a) {
		return a.m_hasValue;
	}

	friend inline void swap(Nullable &a, Nullable &b) {
		using std::swap;
		swap(a.m_value, b.m_value);
		swap(a.m_hasValue, b.m_hasValue);
	}
};

} // ZXing
