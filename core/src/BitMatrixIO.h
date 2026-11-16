/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2017 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string>

#include "BitMatrix.h"
#include "ImageView.h"

namespace ZXing {

std::string ToSVG(ImageView iv, bool invert = false);

std::string ToString(const BitMatrix& matrix, bool inverted = false);
std::string ToString(const BitMatrix& matrix, char one, char zero = ' ', bool addSpace = true, bool printAsCString = false);
std::string ToSVG(const BitMatrix& matrix, bool invert = false);
BitMatrix ParseBitMatrix(const std::string& str, char one = 'X', bool expectSpace = true);
void SaveAsPBM(const BitMatrix& matrix, const std::string& filename, int quietZone = 0);

} // ZXing
