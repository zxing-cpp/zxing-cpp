# ZXing-C++ Kotlin/Native Library

## Install

The easiest way to use the library is to fetch if from _mavenCentral_. Simply add

```gradle
implementation("io.github.zxing-cpp:kotlin-native:2.2.1")
```

to your `build.gradle.kts` file in the `dependencies` section of `nativeMain` source set.

## Use

A trivial use case looks like this:

```kotlin
import zxingcpp.BarcodeFormat.EAN13
import zxingcpp.BarcodeFormat.QRCode
import zxingcpp.BarcodeReader
import zxingcpp.ImageFormat
import zxingcpp.ImageView


val barcodeReader = BarcodeReader().apply {
   formats = setOf(EAN13, QRCode)
   tryHarder = true
   maxNumberOfSymbols = 3
   // more options, see documentation
}

@OptIn(ExperimentalUnsignedTypes::class)
fun process(
   data: UByteArray,
   width: Int,
   height: Int,
   imageFormat: ImageFormat = ImageFormat.Lum // ImageFormat.Lum assumes grey scale image data.
) {
   val image = ImageView(data, width, height, imageFormat)
   barcodeReader.read(image).joinToString("\n") { barcode: Barcode ->
      "${barcode.format} (${barcode.contentType}): ${barcode.text}"
   }
}
```

Here you have to load your image into memory by yourself and pass the decoded data to the constructor of `ImageView`

## Build locally

1. Install JDK, CMake and Android NDK(With `$ANDROID_NDK` correctly configured) and ensure their
   executable binaries appear in `$PATH`.
2. Prepare kotlin/native toolchain (You can easily do this by cloning
   [K/N Toolchain Initializer](https://github.com/ISNing/kn-toolchain-initializer) and executing `gradle build`
   so that kotlin will download toolchains needed into user's home dir.).
3. Ensure there's `llvm-ar` available in `$PATH` for mingwX64 target building.

And then you can build the project from the command line:

	$ ./gradlew :assemble
