# WebAssembly/WASM Wrapper/Library

## Build

1. [Install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/) if not done already.
2. In an empty build folder, invoke `emcmake cmake <path to zxing-cpp.git/wrappers/wasm>`.
3. Invoke `cmake --build .` to create `zxing.js` and `zxing.wasm` (and `_reader`/`_writer` versions).
4. To see how to include these into a working HTML page, have a look at the [reader](demo_reader.html) and [writer](demo_writer.html) demos.
5. To quickly test your build, copy those demo files into your build directory and run e.g. `emrun --serve_after_close demo_reader.html`.

You can also download the latest build output from the continuous integration system from the [Actions](https://github.com/zxing-cpp/zxing-cpp/actions) tab. Look for 'wasm-artifacts'. Also check out the [live demos](https://zxing-cpp.github.io/zxing-cpp/).
