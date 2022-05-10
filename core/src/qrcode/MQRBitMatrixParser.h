#pragma once
/*
 * Copyright 2016 Nu-book Inc.
 * Copyright 2016 ZXing authors
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

class BitMatrix;
class ByteArray;

namespace QRCode {
class FormatInformation;
class Version;
}

namespace MicroQRCode {

/**
 * @brief Reads version information from the micro QR Code.
 * @return {@link Version} encapsulating the micro QR Code's version, nullptr if neither location can be parsed
 */
const QRCode::Version* ReadVersion(const BitMatrix& bitMatrix);

/**
 * @brief Reads format information from one of its two locations within the micro QR Code.
 * @return {@link FormatInformation} encapsulating the micro QR Code's format info, result is invalid if both format
 * information locations cannot be parsed as the valid encoding of format information
 */
QRCode::FormatInformation ReadFormatInformation(const BitMatrix& bitMatrix, bool mirrored);

/**
 * @brief Reads the codewords from the BitMatrix.
 * @return bytes encoded within the micro QR Code or empty array if the exact number of bytes expected is not read
 */
ByteArray ReadCodewords(const BitMatrix& bitMatrix, const QRCode::Version& version,
						const QRCode::FormatInformation& formatInformation, bool mirrored);

} // namespace MicroQRCode
} // namespace ZXing
