/*
 * Copyright 2022 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

namespace ZXing {

class Error
{
public:
	enum class Type { None, Format, Checksum, Unsupported };
	Type type() const noexcept { return _type; }
	const std::string& msg() const noexcept { return _msg; }
	const std::string& location() const noexcept { return _location; }
	explicit operator bool() const noexcept { return _type != Type::None; }

	Error() = default;
	Error(Type type, std::string msg = {}) : _type(type), _msg(std::move(msg)) {}
	Error(std::string file, int line, Type type, std::string msg = {})
		: _type(type), _msg(std::move(msg)), _location(file.substr(file.find_last_of("/\\") + 1) + ":" + std::to_string(line))
	{}

	static constexpr auto Format = Type::Format;
	static constexpr auto Checksum = Type::Checksum;
	static constexpr auto Unsupported = Type::Unsupported;

protected:
	Type _type = Type::None;
	std::string _msg;
	std::string _location;
};

inline bool operator==(const Error& e, Error::Type t) noexcept { return e.type() == t; }
inline bool operator!=(const Error& e, Error::Type t) noexcept { return !(e == t); }
inline bool operator==(Error::Type t, const Error& e) noexcept { return e.type() == t; }
inline bool operator!=(Error::Type t, const Error& e) noexcept { return !(t == e); }

#define FormatError(...) Error(__FILE__, __LINE__, Error::Format, std::string(__VA_ARGS__))
#define ChecksumError(...) Error(__FILE__, __LINE__, Error::Checksum, std::string(__VA_ARGS__))
#define UnsupportedError(...) Error(__FILE__, __LINE__, Error::Unsupported, std::string(__VA_ARGS__))

inline std::string ToString(const Error& e)
{
	const char* name[] = {"", "FormatError", "ChecksumError", "Unsupported"};
	std::string ret = name[static_cast<int>(e.type())];
	if (!e.msg().empty())
		ret += " (" + e.msg() + ")";
	if (!e.location().empty())
		ret += " @ " + e.location();
	return ret;
}

}
