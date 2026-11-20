// SPDX-License-Identifier: Apache-2.0

#include "WXPayload.h"

#include "WXConstants.h"

#include <string_view>

namespace ZXing::WXCode {

// Preserve the streaming radix conversion's low-digit emission order.
// This byte-reservoir algorithm differs from conventional base conversion.
bool TransformPayload(ByteArray& payload)
{
	static constexpr std::string_view TARGET_ALPHABET =
		"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$&'()*+,/:;=?@-._~";
	if (payload.size() > MAX_PAYLOAD_BYTES)
		return false;
	/* FNC1 can introduce bytes outside the compression alphabet. */
	for (uint8_t byte : payload)
		if (ALPHANUMERIC_ALPHABET.find(static_cast<char>(byte)) == std::string_view::npos)
			return true;

	ByteArray intermediate;
	unsigned leadingZeroCount = 0;
	unsigned reservoir = 0;
	bool started = false;
	for (uint8_t byte : payload) {
		auto digit = static_cast<unsigned>(ALPHANUMERIC_ALPHABET.find(static_cast<char>(byte)));
		if (!started && !digit) {
			++leadingZeroCount;
			continue;
		}
		started = true;
		reservoir = reservoir * ALPHANUMERIC_ALPHABET.size() + digit;
		while (reservoir > 255) {
			intermediate.push_back(static_cast<uint8_t>(reservoir));
			reservoir >>= 8;
		}
	}
	if (reservoir)
		intermediate.push_back(static_cast<uint8_t>(reservoir));
	// Consume the emitted bytes in reverse; leading zeros are restored separately.
	ByteArray output;
	reservoir = 0;
	while (!intermediate.empty()) {
		reservoir = reservoir * 256 + intermediate.back();
		intermediate.pop_back();
		while (reservoir >= TARGET_ALPHABET.size()) {
			if (output.size() == MAX_PAYLOAD_BYTES)
				return false;
			output.push_back(static_cast<uint8_t>(TARGET_ALPHABET[reservoir % TARGET_ALPHABET.size()]));
			reservoir /= TARGET_ALPHABET.size();
		}
	}
	if (reservoir) {
		if (output.size() == MAX_PAYLOAD_BYTES)
			return false;
		output.push_back(static_cast<uint8_t>(TARGET_ALPHABET[reservoir]));
	}
	if (leadingZeroCount + output.size() > MAX_PAYLOAD_BYTES)
		return false;
	payload.assign(leadingZeroCount, '0');
	payload.insert(payload.end(), output.rbegin(), output.rend());
	return true;
}

} // namespace ZXing::WXCode
