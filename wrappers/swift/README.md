# Swift Wrapper

This wrapper supports two build/link modes through the root SwiftPM manifest at [Package.swift](../../Package.swift).

## Build Modes

### 1) Bundled source build (default)

Builds zxing-cpp core from source as part of the package.
No preinstalled `libZXing` is required.

```bash
swift build
```

Equivalent explicit form:

```bash
ZXING_BUNDLED=1 swift build
```

### 2) External native library

Links against an already available native `ZXing` library.
Use this when you provide `libZXing` via your own build/distribution system (e.g. homebrew).

```bash
ZXING_BUNDLED=0 swift build -Xcc `pkgconf --cflags zxing` -Xlinker `pkgconf --libs-only-L zxing`
```

## Demo Targets

From repository root:

```bash
swift run demo_writer "Hello World" QRCode out.png
swift run demo_reader out.png
```

For external mode demos, prefix commands with `ZXINGCPP_BUNDLED=0` and pass linker paths as needed.

```bash
ZXING_BUNDLED=0 swift run -Xcc `pkgconf --cflags zxing` -Xlinker `pkgconf --libs-only-L zxing` demo_writer "Hello World" QRCode out.png

ZXING_BUNDLED=0 swift run -Xcc `pkgconf --cflags zxing` -Xlinker `pkgconf --libs-only-L zxing` demo_reader out.png
```
