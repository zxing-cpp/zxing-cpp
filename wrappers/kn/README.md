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

val options = ReaderOptions()
val barcodeReader = BarcodeReader(options)

fun process(image: ImageView) {
    barcodeReader.readBarcodes(image).joinToString("\n") { result ->
        "${result.format} (${result.contentType}): ${result.text}"
    }
}
```

Here you have to implement the `ImageView` class by yourself. Which means you have to decode the image in your own way
and then pass the decoded image wrapped in `ImageView` to the `BarcodeReader` class.

## Build locally

1. Install JDK, CMake and Android NDK(With `$ANDROID_NDK` correctly configured) and ensure their
   executable binaries appear in `$PATH`.
2. Prepare kotlin/native toolchain (You can easily do this by build
   [K/N Toolchain Initializer](https://github.com/ISNing/kn-toolchain-initializer) for once).
3. Ensure there's `llvm-ar` available in `$PATH` for mingwX64 target building.

And then you can build the project from the command line:

	$ ./gradlew :assemble
