# zxing-cpp
Go wrapper for zxing-cpp version 2.3.0

## Prerequisites

This package requires the ZXing C++ library to be installed with pkg-config support.
It depends on the C-API of zxing-cpp.

### Installation

**Ubuntu/Debian:**
```bash
# at build time:
sudo apt install libzxing-dev
# at run time libzxing3 is enough
sudo apt install libzxing3
```

**From source:**
```bash
git clone https://github.com/zxing-cpp/zxing-cpp.git --recursive --single-branch --depth 1 --branch v2.3.0
cd zxing-cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DZXING_C_API=ON
cmake --build build
sudo cmake --install build
sudo ldconfig

# Create pkg-config file
sudo tee /usr/local/lib/pkgconfig/zxing.pc > /dev/null <<EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: ZXing
Description: ZXing-C++ library
Version: 2.3.0
Libs: -L\${libdir} -lZXing
Cflags: -I\${includedir} -I\${includedir}/ZXing
EOF

# Update pkg-config path (add to ~/.bashrc for persistence)
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

Verify installation:
```bash
pkg-config --cflags --libs zxing
```

## Installation

```bash
go get github.com/Mittelstand-ai-GmbH-Co-KG/zxing-cpp
```

## Usage

```go
package main

import (
    "fmt"
    "image/png"
    "os"
    
    "github.com/Mittelstand-ai-GmbH-Co-KG/zxing-cpp"
)

func main() {
    // Open image file
    file, err := os.Open("barcode.png")
    if err != nil {
        panic(err)
    }
    defer file.Close()
    
    // Decode image
    img, err := png.Decode(file)
    if err != nil {
        panic(err)
    }
    
    // Create config and scan
    config := zxing.NewConfig()
    if err != nil {
		panic(err)
	}
	defer config.Close()

    symbols, err := config.Scan(img)
    if err != nil {
        panic(err)
    }
    
    // Print results
    for _, symbol := range symbols {
        fmt.Printf("Type: %s\nData: %s\n", symbol.BcType, symbol.Data)
    }
}
```

Build with `CGO_ENABLED`. For example

```bash
CGO_ENABLED=1 go build -installsuffix 'static' -o /app .
```

## Supported image types

Supported image types are

`image.Gray`, `image.RGBA`, `image.NRGBA` and `image.YCbCr`.
