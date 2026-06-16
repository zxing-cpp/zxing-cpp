# librscpp

`librscpp` is a small header-only Reed-Solomon implementation in C++20.

Pick a Galois field that matches your symbology or protocol, then call `encode()` or `decode()` on a codeword.

## Example

```cpp
#include "librscpp/encode.h"
#include "librscpp/decode.h"

#include <string>
#include <print>

int main()
{
	// GF(256) used in QR Codes with primitive = x^8 + x^4 + x^3 + x^2 + 1 and fcr = 0.
	auto field = librscpp::GF2n<uint8_t>(8, 0b1'0001'1101, 0);

	const std::string data = "Hello, World!";
	constexpr int paritySize = 4;

	auto codeword = librscpp::encode(field, data, paritySize);
	std::println("Encoded codeword:  '{}'", codeword);

	// Simulate a couple of symbol errors.
	codeword[3] = codeword[8] = '*';
	std::println("Received codeword: '{}'", codeword);

	auto res = librscpp::decode(field, codeword, paritySize);
	if (res)
		std::println("Decoded codeword:  '{}'\nUsed parity symbols: {}", codeword, *res);
	else
		std::println("Failed to decode codeword.");

	return 0;
}
```

Running this example program produces the following output:
```
Encoded codeword:  'Hello, World!??t'
Received codeword: 'Hel*o, W*rld!??t'
Decoded codeword:  'Hello, World!??t'
Used parity symbols: 4
```

## Configuration

The PDF417 symbology uses the prime GF(929) with primitive root 3 and fcr 1: `librscpp::GFp<>(929, 3, 1)`.

If you are low on memory, you can `#define LIBRSCPP_SAVE_MEMORY` before including the header to save 1/3 of runtime memory used by the GF internal lookup tables. Without that, each GF2n/GFp instance allocates 3 * field_size * sizeof(T) bytes. Also the default `value_type` of `GF2n<>` and `GFp<>` is `uint16_t`. If your field size is <=256 (like the one in the sample code), you can save 50% by using `uint8_t`.

Other common/usable `GF2n` configurations are:
```c++
	GF2n<> gf_0x0013(4, 0x0013, 1);  // x^4 + x + 1
	GF2n<> gf_0x0043(6, 0x0043, 1);  // x^6 + x + 1
	GF2n<> gf_0x012D(8, 0x012D, 1);  // x^8 + x^5 + x^3 + x^2 + 1 : DataMatrix
	GF2n<> gf_0x011D(8, 0x011D, 0);  // x^8 + x^4 + x^3 + x^2 + 1 : QRCode
	GF2n<> gf_0x0409(10, 0x0409, 1); // x^10 + x^3 + 1
	GF2n<> gf_0x1069(12, 0x1069, 1); // x^12 + x^6 + x^5 + x^3 + 1
```