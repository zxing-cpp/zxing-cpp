/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>
#include <set>
#include <string>

namespace fs = std::filesystem;

namespace ZXing::Test {

int runBlackBoxTests(const fs::path& testPathPrefix, const std::set<std::string>& includedTests);

} // ZXing::Test
