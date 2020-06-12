#pragma once
/*
* Copyright 2020 Axel Waggershauser
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

#include <type_traits>

namespace ZXing {

template<typename Enum>
class ZXFlags
{
	static_assert(std::is_enum<Enum>::value, "ZXFlags is only usable on enumeration types.");

	using Int = typename std::underlying_type<Enum>::type;
	Int i = 0;

	constexpr inline ZXFlags(Int other) : i(other) {}

public:
	using enum_type = Enum;

	constexpr inline ZXFlags() noexcept = default;
	constexpr inline ZXFlags(Enum flag) noexcept : i(Int(flag)) {}

//	constexpr inline ZXFlags(std::initializer_list<Enum> flags) noexcept
//		: i(initializer_list_helper(flags.begin(), flags.end()))
//	{}

	constexpr inline bool operator==(ZXFlags other) const noexcept { return i == other.i; }

	inline ZXFlags& operator&=(ZXFlags mask) noexcept { return i &= mask.i, *this; }
	inline ZXFlags& operator&=(Enum mask) noexcept { return i &= Int(mask), *this; }
	inline ZXFlags& operator|=(ZXFlags other) noexcept { return i |= other.i, *this; }
	inline ZXFlags& operator|=(Enum other) noexcept { return i |= Int(other), *this; }
//	inline ZXFlags &operator^=(ZXFlags other) noexcept { return i ^= other.i, *this; }
//	inline ZXFlags &operator^=(Enum other) noexcept { return i ^= Int(other), *this; }

	constexpr inline ZXFlags operator&(ZXFlags other) const noexcept { return i & other.i; }
	constexpr inline ZXFlags operator&(Enum other) const noexcept { return i & Int(other); }
	constexpr inline ZXFlags operator|(ZXFlags other) const noexcept { return i | other.i; }
	constexpr inline ZXFlags operator|(Enum other) const noexcept { return i | Int(other); }
//	constexpr inline ZXFlags operator^(ZXFlags other) const noexcept { return i ^ other.i; }
//	constexpr inline ZXFlags operator^(Enum other) const noexcept { return i ^ Int(other); }
//	constexpr inline ZXFlags operator~() const noexcept { return ~i; }

//	constexpr inline operator Int() const noexcept { return i; }
//	constexpr inline bool operator!() const noexcept { return !i; }
	constexpr inline Int asInt() const noexcept { return i; }

	constexpr inline bool testFlag(Enum flag) const noexcept
	{
		return (i & Int(flag)) == Int(flag) && (Int(flag) != 0 || i == Int(flag));
	}
	inline ZXFlags& setFlag(Enum flag, bool on = true) noexcept
	{
		return on ? (*this |= flag) : (*this &= ~Int(flag));
	}
	inline ZXFlags clear() noexcept
	{
		i = 0;
		return *this;
	}

private:
//	constexpr static inline Int
//	initializer_list_helper(typename std::initializer_list<Enum>::const_iterator it,
//							typename std::initializer_list<Enum>::const_iterator end) noexcept
//	{
//		return (it == end ? Int(0) : (Int(*it) | initializer_list_helper(it + 1, end)));
//	}
};

#define ZX_DECLARE_FLAGS(FLAGS, ENUM) \
	using FLAGS = ZXFlags<ENUM>; \
	constexpr inline FLAGS operator|(FLAGS::enum_type e1, FLAGS::enum_type e2) noexcept { return FLAGS(e1) | e2; } \
	constexpr inline FLAGS operator|(FLAGS::enum_type e, FLAGS f) noexcept { return f | e; } \
	constexpr inline bool operator==(FLAGS::enum_type e, FLAGS f) noexcept { return FLAGS(e) == f; } \
	constexpr inline bool operator==(FLAGS f, FLAGS::enum_type e) noexcept { return FLAGS(e) == f; }

} // ZXing

