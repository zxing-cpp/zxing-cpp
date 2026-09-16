/*
 * Copyright 2022 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "Error.h"
#include "ZXAlgorithms.h"

namespace ZXing {

std::string Error::location() const
{
	if (!_file)
		return {};
	std::string file(_file);
	return file.substr(file.find_last_of("/\\") + 1) + ":" + std::to_string(_line);
}

std::string ToString(const Error& e)
{
	std::string ret = EnumToString(e.type(), {"", "FormatError", "ChecksumError", "Unsupported"});
	if (!e.msg().empty())
		ret += " (" + e.msg() + ")";
	if (auto location = e.location(); !location.empty())
		ret += " @ " + e.location();
	return ret;
}

}
