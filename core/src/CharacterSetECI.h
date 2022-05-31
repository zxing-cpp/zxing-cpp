/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "CharacterSet.h"

#include <string>

namespace ZXing {

/**
* Encapsulates a Character Set ECI, according to AIM ITS/04-023:2004 Extended Channel Interpretations Part 3: Register
*
* @author Sean Owen
*/
namespace CharacterSetECI {

/**
 * @param name character set ECI encoding name
 * @return {@code CharacterSet} representing ECI of given value, or {@code CharacterSet::Unknown} if it is
 *   unsupported
 */
CharacterSet CharsetFromName(const char* name);

} // namespace CharacterSetECI
} // namespace ZXing
