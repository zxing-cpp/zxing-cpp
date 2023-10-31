# WebAssembly/WASM Wrapper

## Build

1. [Install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/) if not done already.
2. In an empty build folder, invoke `emcmake cmake <path to zxing-cpp.git/wrappers/wasm>`.
3. Invoke `cmake --build .` to create `zxing.js` and `zxing.wasm` (and `_reader`/`_writer` versions).
4. To see how to include these into a working HTML page, have a look at the [reader](demo_reader.html), [writer](demo_writer.html) and [cam reader](demo_cam_reader.html) demos.
5. To quickly test your build, copy those demo files into your build directory and run e.g. `emrun --serve_after_close demo_reader.html`.

You can also download the latest build output from the continuous integration system from the [Actions](https://github.com/zxing-cpp/zxing-cpp/actions) tab. Look for 'wasm-artifacts'. Also check out the [live demos](https://github.com/zxing-cpp/zxing-cpp#web-demos).

## Alternative Wrapper Project

There is an alternative (external) wrapper project called [zxing-wasm](https://github.com/Sec-ant/zxing-wasm). It is written in TypeScript, has a more feature complete interface closer to the C++ API, spares you from dealing with WASM intricacies and is provided as a fully fledged ES module on [npmjs](https://www.npmjs.com/package/zxing-wasm).

## Performance

It turns out that compiling the library with the `-Os` (`MinSizeRel`) flag causes a noticible performance penalty. Here are some measurements from the demo_cam_reader (performed on Chromium 109 running on a Core i9-9980HK):

|         | `-Os` | `-Os -flto` | `-O3`  | `-O3 -flto` | _Build system_ |
|---------|-------|-------------|--------|-------------|-|
| size    | 790kB | 950kb       | 940kb  | 1000kB      | _All_               |
| runtime | 320ms | 30ms        | 8ms    | 8ms         | C++17, emsdk 3.1.9  |
| runtime | 13ms  | 30ms        | 8ms    | 8ms         | C++17, emsdk 3.1.31 |
| runtime | 46ms  | 46ms        | 11ms   | 11ms        | C++20, emsdk 3.1.31 |

Conclusions:
 * saving 15% of download size for the price of a 2x-4x slowdown seems like a hard sale (let alone the 40x one)...
 * building in C++-20 mode brings position independent DataMatrix detection but costs 35% more time
 * link time optimization (`-flto`) is not worth it and potentially even counter productive
 
