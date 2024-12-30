# ZXing-C++ Kotlin/Native Library

## Install

The easiest way to use the library is to fetch it from _mavenCentral_. Simply add

```gradle
implementation("io.github.zxing-cpp:kotlin-native:2.3.0-SNAPSHOT")
```

to your `build.gradle.kts` file in the `dependencies` section of `nativeMain` source set.
To access the SNAPSHOT version, you also need to add a separate repositories entry in your build.cradle file:

```gradle
maven { url = uri("https://s01.oss.sonatype.org/content/repositories/snapshots") }
```

## Use

### Reading

A trivial use case looks like this:

```kotlin
import zxingcpp.BarcodeFormat
import zxingcpp.BarcodeReader
import zxingcpp.ImageFormat
import zxingcpp.ImageView

val data: ByteArray = ...    // the image data
val width: Int = ...         // the image width
val height: Int = ...        // the image height
val format: ImageFormat = ImageFormat.Lum // ImageFormat.Lum assumes grey scale image data

val image: ImageView = ImageView(data, width, height, format)
val barcodeReader = BarcodeReader().apply {
   formats = setOf(BarcodeFormat.EAN13, BarcodeFormat.QRCode)
   tryHarder = true
   maxNumberOfSymbols = 3
   // more options, see documentation
}

barcodeReader.read(image).joinToString("\n") { barcode: Barcode ->
   "${barcode.format} (${barcode.contentType}): ${barcode.text}"
}
```

Here you have to load your image into memory by yourself and pass the decoded data to the constructor of `ImageView`.

### Writing

A trivial use case looks like this:

```kotlin
import zxingcpp.*

val text: String = "Hello, World!"
val format = BarcodeFormat.QRCode

@OptIn(ExperimentalWriterApi::class)
val cOpts = CreatorOptions(format) // more options, see documentation

@OptIn(ExperimentalWriterApi::class)
val barcode = Barcode(text, cOpts)
// or
@OptIn(ExperimentalWriterApi::class)
val barcode2 = Barcode(text.encodeToByteArray(), format)

@OptIn(ExperimentalWriterApi::class)
val wOpts = WriterOptions().apply {
   sizeHint = 400
   // more options, see documentation
}

@OptIn(ExperimentalWriterApi::class)
val svg: String = barcode.toSVG(wOpts)
@OptIn(ExperimentalWriterApi::class)
val image: Image = barcode.toImage(wOpts)
```

> Note: The Writer api is still experimental and may change in future versions.
> You will have to opt-in `zxingcpp.ExperimentalWriterApi` to use it.

## Build locally

1. Install JDK, CMake and Android NDK(With `$ANDROID_NDK` correctly configured) and ensure their
   executable binaries appear in `$PATH`.
2. Prepare kotlin/native toolchain (You can easily do this by cloning
   [K/N Toolchain Initializer](https://github.com/ISNing/kn-toolchain-initializer) and executing `gradle build`
   so that kotlin will download toolchains needed into user's home dir.).
3. Ensure there's `run_konan` available in `$PATH` or specify path of kotlin-native toolchain in `local.properties`
   like `konan.dir=/home/user/.konan/kotlin-native-prebuilt-linux-x86_64-1.9.22`.
4. Ensure there's `llvm-ar` available in `$PATH` for mingwX64 target building.

And then you can build the project from the command line:

	$ ./gradlew :assemble
