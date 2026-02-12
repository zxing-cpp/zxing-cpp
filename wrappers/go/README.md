# Go wrapper for zxing-cpp

This folder contains a cgo-based Go wrapper around the zxing-cpp **C API** (`core/src/ZXingC.h`).

The wrapper links against the native `ZXing` shared library built by this repo's CMake.

This wrapper must be considered experimental at this point. The API might change anytime. Suggestions for improvements are welcome.

## Installation

The wrapper supports two build modes:

1. Installed mode (default): uses `pkg-config` (`zxing.pc`) and works with system packages.
2. Local mode: links directly against an in-tree CMake build (useful for CI).

### Installed mode (default)

Install zxing-cpp locally or as package first, for example:

- macOS: `brew install zxing-cpp`
- Debian/Ubuntu: `apt install zxing-cpp`

Then run:

```sh
cd wrappers/go
go test ./...
```

### Local mode (no install, CI-friendly)

Build zxing-cpp from the repo root:

```sh
cmake -S . -B build
cmake --build build
```

Then run Go with the local link mode enabled:

```sh
cd wrappers/go
go test -tags zxing_local ./...
```

This mode expects the native library in `build/core` and headers in `core/src`.

In your Go code:

```go
import "github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
```

Minimal reader example:

```go
package main

import (
	"fmt"
	"image"
	"os"
	_ "image/jpeg"
	_ "image/png"

	"github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
)

func main() {
	f, err := os.Open("in.png")
	if err != nil {
		panic(err)
	}
	defer f.Close()

	img, _, err := image.Decode(f)
	if err != nil {
		panic(err)
	}

	barcodes, err := zxingcpp.ReadBarcodes(img)
	if err != nil {
		panic(err)
	}

	for _, bc := range barcodes {
		fmt.Println(bc.Format(), bc.Text())
		bc.Close()
	}
}
```

Minimal writer example:

```go
bc, _ := zxingcpp.CreateBarcode("hello", zxingcpp.BarcodeFormatQRCode)
defer bc.Close()

img, _ := bc.ToImage(zxingcpp.WithScale(4)) // returns standard image.Image
svg, _ := bc.ToSVG(zxingcpp.WithHRT(true))  // returns string
_ = img
_ = svg
```

## Demo programs

From `wrappers/go`:

```sh
# writer
go run ./demo/demo_writer TEXT FORMAT OUTFILE(.svg|.png)

# reader
go run ./demo/demo_reader IMAGE [FORMATS] [fast]
```

## Notes

- `ImageView` is a non-owning view in the C++ API.
- `ReadBarcodes` accepts either `image.Image` or `*ImageView`.
- `ToImage` returns a standard Go `image.Image` (`*image.Gray` currently), not a wrapper around C memory.
- For advanced/zero-copy use cases, `NewImageViewFromC` can wrap a pointer to **C-allocated** memory.

