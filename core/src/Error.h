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
	enum Type { None, Format, Checksum, Unsupported };
	Type type() const noexcept { return _type; }
	const std::string& msg() const noexcept { return _msg; }
	operator bool() const noexcept { return _type != None; }

	Error() = default;

protected:
	Type _type = None;
	std::string _msg;

	Error(Type type, std::string msg) : _type(type), _msg(std::move(msg)) {}
};

class FormatError : public Error
{
public:
	FormatError(std::string msg = {}) : Error(Format, std::move(msg)) {}
};

class ChecksumError : public Error
{
public:
	ChecksumError(std::string msg = {}) : Error(Checksum, std::move(msg)) {}
};

inline std::string ToString(const Error& e)
{
	const char* name[] = {"", "FormatError", "ChecksumError", "Unsupported"};
	std::string ret = name[static_cast<int>(e.type())];
	if (!e.msg().empty())
		ret += " (" + e.msg() + ")";
	return ret;
}

}
