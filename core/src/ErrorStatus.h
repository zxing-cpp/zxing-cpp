#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

namespace ZXing {

enum class ErrorStatus
{
	NoError = 0,

	ReaderError = 0x10,
	NotFound,
	FormatError,
	ChecksumError,

	ReedSolomonError = 0x20,
	ReedSolomonAlgoFailed,		// r_{i-1} was zero
	ReedSolomonBadLocation,		// Bad error location
	ReedSolomonDegreeMismatch,	// Error locator degree does not match number of roots
	ReedSolomonSigmaTildeZero,	// sigmaTilde(0) was zero
};

inline bool StatusIsOK(ErrorStatus status)
{
	return status == ErrorStatus::NoError;
}

inline bool StatusIsError(ErrorStatus status)
{
	return status != ErrorStatus::NoError;
}

bool StatusIsKindOf(ErrorStatus status, ErrorStatus group);

} // ZXing
