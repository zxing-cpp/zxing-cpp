/*
* Copyright 2022 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

// see http://google.github.io/googletest/advanced.html#sanitizer-integration

extern "C"
{
	void __ubsan_on_report() { FAIL() << "Encountered an undefined behavior sanitizer error"; }
	void __asan_on_error() { FAIL() << "Encountered an address sanitizer error"; }
	void __tsan_on_report() { FAIL() << "Encountered a thread sanitizer error"; }
} // extern "C"
