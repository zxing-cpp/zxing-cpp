/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ZXFilesystem.h"

#include <set>
#include <string>

namespace ZXing::Test {

int runBlackBoxTests(const fs::path& blackboxPath, const std::set<std::string>& includedTests);

} // ZXing::Test
