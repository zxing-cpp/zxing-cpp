# librscpp

`librscpp` is a small header-only Reed-Solomon implementation in c++20.

Pick a Galois field that matches your symbology or protocol,
then call `encode()` or `decode()` on a message buffer in place.

## Encoding and decoding

This example uses the QR Code field GF(256) with a fcr/base of 0.

```cpp
#include "librscpp/encode.h"
#include "librscpp/decode.h"

#include <string>
#include <print>

int main()
{
	auto field = librscpp::GF2n<>(8, 0b1'0001'1101, 0); // QR Code: x^8 + x^4 + x^3 + x^2 + 1

	std::string message = "Hello, World!####";
	constexpr int ecWords = 4;

	// Encode parity words in place.
	librscpp::encode(field, message, ecWords);
	std::println("Encoded message:  '{}'", message);

	// Simulate a couple of symbol errors.
	message[3] = message[8] = '*';
	std::println("Received message: '{}'", message);

	auto res = librscpp::decode(field, message, ecWords);
	if (res)
		std::println("Decoded message:  '{}'\nUsed ECC codewords: {}", message, *res);
	else
		std::println("Failed to decode message.");

	return 0;
}
```

For other formats, instantiate a different field, for example `librscpp::GFp<>(929, 3, 1)` for PDF417.

If you are low on memory, you can `#define LIBRSCPP_SAVE_MEMORY` before including the header to save 1/3 of runtime memory.
Without that, each GF2n/GFp instance allocates 3 * field_size * sizeof(T) bytes.
