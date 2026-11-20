// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ByteArray.h"

namespace ZXing::WXCode {

// Convert the WX alphanumeric payload in place. Bytes outside its compression
// alphabet are preserved for FNC1 handling. Four versions need at most 160 bytes.
bool TransformPayload(ByteArray& payload);

} // namespace ZXing::WXCode
