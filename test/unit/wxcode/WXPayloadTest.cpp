// SPDX-License-Identifier: Apache-2.0

#include "wxcode/WXPayload.h"

#include "gtest/gtest.h"
#include <cstdint>

using namespace ZXing::WXCode;

TEST(WXCodePayloadTest, NativeConversionGoldenDigest)
{
	const char* alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";
	uint32_t state = 17;
	uint64_t hash = UINT64_C(14695981039346656037);
	for (int n = 0; n < 1000; ++n) {
		ZXing::ByteArray payload(n % 100);
		for (size_t i = 0; i < payload.size(); ++i) {
			state = state * UINT32_C(1664525) + UINT32_C(1013904223);
			payload[i] = static_cast<uint8_t>(alphabet[state % 45]);
		}
		ASSERT_TRUE(TransformPayload(payload));
		hash = (hash ^ payload.size()) * UINT64_C(1099511628211);
		for (size_t i = 0; i < payload.size(); ++i)
			hash = (hash ^ payload[i]) * UINT64_C(1099511628211);
	}
	// Golden reference from 1000 native outputs; see the WX Code README for provenance.
	EXPECT_EQ(hash, UINT64_C(0x925d9f327fb2b041));
	ZXing::ByteArray payload{'0', '0', '0', '0', '0'};
	ASSERT_TRUE(TransformPayload(payload));
	EXPECT_EQ(payload, ZXing::ByteArray({'0', '0', '0', '0', '0'}));
	payload = {29};
	ASSERT_TRUE(TransformPayload(payload));
	EXPECT_EQ(payload, ZXing::ByteArray({29}));
}

TEST(WXCodePayloadTest, CapacityFailurePreservesInput)
{
	ZXing::ByteArray payload;
	payload.assign(161, 'A');
	auto original = payload;
	EXPECT_FALSE(TransformPayload(payload));
	EXPECT_EQ(payload, original);
}
