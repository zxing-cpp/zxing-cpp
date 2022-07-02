/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace ZXing {

enum class DecodeStatus
{
	NoError = 0,
	NotFound,
	FormatError,
	ChecksumError,
};

[[deprecated]] inline bool StatusIsOK(DecodeStatus status)
{
	return status == DecodeStatus::NoError;
}

[[deprecated]] inline bool StatusIsError(DecodeStatus status)
{
	return status != DecodeStatus::NoError;
}

[[deprecated]] inline const char* ToString(DecodeStatus status)
{
	constexpr const char* names[] = {"NoError", "NotFound", "FormatError", "ChecksumError"};
	return names[static_cast<int>(status)];
}

} // ZXing
