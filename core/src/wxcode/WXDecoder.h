// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"
#include "WXGrid.h"

namespace ZXing::WXCode {

// Decode a canonical angular/radial grid of binary cells. Failure returns empty.
ByteArray Decode(const Grid& grid);

} // namespace ZXing::WXCode
