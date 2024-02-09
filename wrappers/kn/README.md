# ZXing-C++ Kotlin/Native Library

## Install

The easiest way to use the library is to fetch if from _mavenCentral_. Simply add

```gradle
implementation("io.github.zxing-cpp:kotlin-native:2.2.1")
```

to your `build.gradle.kts` file in the `dependencies` section of `nativeMain` source set.

## Use

A trivial use case looks like this (in Kotlin):

```kotlin
import zxingcpp.BarcodeReader
import zxingcpp.ImageView

val barcodeReader = BarcodeReader()

fun process(image: ImageView) {
    barcodeReader.readBarcodes(image).joinToString("\n") { result ->
        "${result.format} (${result.contentType}): ${result.text}"
    }
}
```

## Build locally

1. Install JDK, CMake, LLVM(With Clang and LLD), Android NDK(With `$ANDROID_NDK` correctly configured) and ensure their
   executable binaries
   appear in `$PATH`.
2. Prepare gcc toolchain for `aarch64-linux-gnu`, `x86_64-linux-gnu`, if clang doesn't find them correctly, you can
   define sysroot/gcc toolchain path in `local.properties` by `linux{Arm64,X64}.{sysRoot,gccToolchain}`

And then you can build the project from the command line:

	$ ./gradlew :zxingcpp:assemble
