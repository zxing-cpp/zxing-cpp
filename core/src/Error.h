/*
 * Copyright 2022 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>
#include <cstdint>

namespace ZXing {

class Error
{
public:
	enum class Type : uint8_t { None, Format, Checksum, Unsupported };
	Type type() const noexcept { return _type; }
	const std::string& msg() const noexcept { return _msg; }
	explicit operator bool() const noexcept { return _type != Type::None; }

	std::string location() const
	{
		if (!_file)
			return {};
		std::string file(_file);
		return file.substr(file.find_last_of("/\\") + 1) + ":" + std::to_string(_line);
	}

	Error() = default;
	Error(Type type, std::string msg = {}) : _msg(std::move(msg)), _type(type) {}
	Error(const char* file, short line, Type type, std::string msg = {}) : _msg(std::move(msg)), _file(file), _line(line), _type(type) {}

	static constexpr auto Format = Type::Format;
	static constexpr auto Checksum = Type::Checksum;
	static constexpr auto Unsupported = Type::Unsupported;

	inline bool operator==(const Error& o) const noexcept
	{
		return _type == o._type && _msg == o._msg && _file == o._file && _line == o._line;
	}
	inline bool operator!=(const Error& o) const noexcept { return !(*this == o); }

protected:
	std::string _msg;
	const char* _file = nullptr;
	short _line = -1;
	Type _type = Type::None;
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
	if (auto location = e.location(); !location.empty())
		ret += " @ " + e.location();
	return ret;
}

}
