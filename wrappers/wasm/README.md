# WebAssembly/WASM Wrapper

## Build

1. [Install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/) if not done already.
2. In an empty build folder, invoke `emcmake cmake <path to zxing-cpp.git/wrappers/wasm>`.
3. Invoke `cmake --build .` to create `zxing.js` and `zxing.wasm` (and `_reader`/`_writer` versions).
4. To see how to include these into a working HTML page, have a look at the [reader](demo_reader.html), [writer](demo_writer.html) and [cam reader](demo_cam_reader.html) demos.
5. To quickly test your build, copy those demo files into your build directory and run e.g. `emrun --serve_after_close demo_reader.html`.

You can also download the latest build output from the continuous integration system from the [Actions](https://github.com/zxing-cpp/zxing-cpp/actions) tab. Look for 'wasm-artifacts'. Also check out the [live demos](https://github.com/zxing-cpp/zxing-cpp#web-demos).

## Reducing the Download Size

These CMake options let applications choose which wrapper features and optimizations to include:

| Option | Effect |
|--------|--------|
| `-DZXING_WASM_IMAGE_DECODING=OFF` | Omits `readBarcode(s)FromImage` and stb_image decoding. Use `readBarcode(s)FromPixmap` with pixel data decoded by the browser or application instead. |
| `-DZXING_ENABLE_UNICODE=OFF` | Omits support for multi-byte character sets and less common single-byte code pages. ASCII, UTF-8, ISO-8859-1, and binary data remain supported; text in omitted character sets is returned as bytes without decoded text. |
| `-DZXING_WASM_EXCEPTIONS=wasm` | Uses native WebAssembly exceptions instead of JavaScript exception emulation. This requires WebAssembly exception handling support in the target runtime; the default `js` mode has broader compatibility. |
| `-DZXING_ENABLE_<FORMAT>=OFF` | Drops support for a format family: `1D`, `AZTEC`, `DATAMATRIX`, `MAXICODE`, `PDF417`, `QRCODE`. |
| `-DZXING_WASM_LTO=OFF` | Disables link time optimization, which is enabled by default. |
| `-DCMAKE_BUILD_TYPE=MinSizeRel` | Optimizes for size (`-Os`) instead of speed (`-O3`). |
| `-DCMAKE_EXE_LINKER_FLAGS=-sMALLOC=emmalloc` | Uses emscripten's smaller (but simpler) memory allocator. |

## Alternative Wrapper Project

There is an alternative (external) wrapper project called [zxing-wasm](https://github.com/Sec-ant/zxing-wasm). It is written in TypeScript, has a more feature complete interface closer to the C++ API, spares you from dealing with WASM intricacies and is provided as a fully fledged ES module on [npmjs](https://www.npmjs.com/package/zxing-wasm).

## Performance

The measurements below were collected locally on an Apple M4 Pro using Chromium 150 and Emscripten 7.0.0-git. Each C++20 build included the reader only, with writers and file-image decoding disabled. Download size is the combined `zxing_reader.js` and `zxing_reader.wasm` size. Runtime is the median of 300 scans, after 30 warmups, of `test/samples/none-2/17.webp` (480x480); Chromium decoded the WebP once, then each scan used `readBarcodesFromPixmap`, including canvas pixel extraction and copying pixels to WASM memory. No barcodes were detected. Results vary by toolchain and device.

|                         | `-Os`  | `-Os -flto` | `-O3`  | `-O3 -flto` |
|-------------------------|--------|-------------|--------|-------------|
| Download size (JS+WASM) | 769 kB | 718 kB      | 890 kB | 869 kB      |
| Median runtime          | 28.5 ms| 23.7 ms     | 11.7 ms| 10.3 ms     |

Conclusions:
 * `-O3` was about 2.4x faster than `-Os` for this image, with about 16% more download size
 * LTO reduced download size and median runtime in both optimization modes in this local test; use `ZXING_WASM_LTO` to compare on your target toolchain and device
 
