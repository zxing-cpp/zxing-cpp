# Rust Wrapper

This crate is a Rust wrapper for the C++ library [zxing-cpp](https://github.com/zxing-cpp/zxing-cpp).

## Requirements

- zxing-cpp >= 2.2.1 (minimum tested)
- C++17 compatible compiler

## Environment Variables

- `ZXING_CPP_LIB_DIR` (Required for binaries): Path to the directory of the ZXing static library built by zxing-cpp. 
                                               This is used during the linking stage when building an executable.
- `ZXING_CPP_SRC_DIR` (Required): Path to the source directory of zxing-cpp. This is used to generate the bindings. 
                                  `<zxing-cpp-repo-path>/core/src`

### Restrictions

This crate uses [autocxx](https://github.com/google/autocxx) to generate the underlying bindings. 

Only reading is currently implemented. See [#332](https://github.com/zxing-cpp/zxing-cpp/issues/332) for the status of 
writing barcodes with zxing-cpp.