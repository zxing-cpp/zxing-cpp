# Go wrapper for zxing-cpp

This folder contains a cgo-based Go wrapper around the zxing-cpp **C API** (`core/src/ZXingC.h`).

The wrapper links against the native `ZXing` shared library built by this repo's CMake.

This wrapper must be considered experimental at this point. The API might change anytime. Suggestions for improvements are welcome.

## Build the native library

From the repo root:

```sh
cmake -S . -B build -DZXING_C_API=ON -DBUILD_SHARED_LIBS=ON
cmake --build build
```

This produces a shared library named like:

- macOS: `build/core/libZXing.dylib` (may have a version suffix)
- Linux: `build/core/libZXing.so`
- Windows: `build/core/ZXing.dll`

## Use from Go

Ensure the dynamic loader can find the native library:

- macOS: `DYLD_LIBRARY_PATH=build/core`
- Linux: `LD_LIBRARY_PATH=build/core`
- Windows: add `build\\core` to `PATH`

Then:

```sh
cd wrappers/go
go test ./...
```

In your Go code:

```go
import "github.com/zxing-cpp/zxing-cpp/wrappers/go/zxingcpp"
```

Minimal usage (from a standard `image.Image`):

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

Minimal usage (from a raw luminance buffer):

```go
iv, err := zxingcpp.NewImageView(data, width, height, zxingcpp.ImageFormatLum, rowStride, pixStride)
if err != nil {
	panic(err)
}
defer iv.Close()

barcodes, err := zxingcpp.ReadBarcodes(iv) // opts are optional
```

Writing:

```go
copts, _ := zxingcpp.NewCreatorOptions(zxingcpp.BarcodeFormatQRCode)
defer copts.Close()

bc, _ := zxingcpp.CreateBarcodeFromText("hello", copts)
defer bc.Close()

wopts, _ := zxingcpp.NewWriterOptions()
defer wopts.Close()
wopts.SetScale(4)

img, _ := bc.ToImage(wopts) // returns standard image.Image
svg, _ := bc.ToSVG(wopts)
_ = img
_ = svg
```

## Demo programs

From `wrappers/go`:

```sh
# reader
go run ./cmd/demo_reader IMAGE [FORMATS] [fast]

# writer
go run ./cmd/demo_writer TEXT FORMAT OUTFILE(.svg|.png)
```

## Notes

- `ImageView` is a non-owning view in the C++ API. To keep the Go wrapper safe with cgo rules, `NewImageView` **copies** the pixel buffer into C-allocated memory and frees it when the `ImageView` is closed.
- `ReadBarcodes` accepts either `image.Image` or `*ImageView`.
- `ToImage` returns a standard Go `image.Image` (`*image.Gray` currently), not a wrapper around C memory.
- For advanced/zero-copy use cases, `NewImageViewFromC` can wrap a pointer to **C-allocated** memory.

