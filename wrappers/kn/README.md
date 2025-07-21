# ZXing-C++ Kotlin/Native Library

## Install

The easiest way to use the library is to fetch it from _mavenCentral_. Simply add

```kotlin
implementation("io.github.zxing-cpp:kotlin-native:2.3.0-SNAPSHOT")
```

to your `build.gradle.kts` file in the `dependencies` section of `nativeMain` source set.
To access the SNAPSHOT version, you also need to add a separate repositories entry in your build.cradle file:

```kotlin
maven { url = uri("https://s01.oss.sonatype.org/content/repositories/snapshots") }
```

### Library distribution

> For further information, see: [zxing-cpp#939](https://github.com/zxing-cpp/zxing-cpp/issues/939)

We currently provide two ways to distribute the native library, depending on the target platform:

#### Static library distribution

This section is suitable for all targets other than `linuxX64` and `linuxArm64`, such as `androidNativeArm64`,
`macosX64`, etc.

The wrapper is distributed with its prebuilt static library in klibs,
so you can use it without any additional configuration when using the library on these targets.

#### Shared library distribution

This section is suitable for only `linuxX64` and `linuxArm64` targets.
The library has to be installed on both dev and client host to ensure the library works.

You need to specify the place of compiled shared library for each target you need for kn's linker like the following
code:

```kotlin
linuxX64.compilations["main"].compileTaskProvider.configure {
    compilerOptions.freeCompilerArgs.addAll(
        "-linker-options",
        "-L/home/user/zxing-cpp/build/install/lib -lZXing"
    )
}
```

Otherwise, when compiling the (consumer)project, you will see the following error:

```text
> Task :linkDebugTestLinuxX64 FAILED
e: .../usr/bin/ld invocation reported errors
...
Proposed solutions:
0. From io.github.zxing-cpp:zxing-cpp-cinterop-libZXing:
Since Kotlin intends to deprecate the ability of including static library into klibs, now the kn wrapper of zxing-cpp no longer provides them in klibs, users will have to handle dynamic library distribution by themselves, for further information, see: https://github.com/zxing-cpp/zxing-cpp/issues/939 .
The .../usr/bin/ld command returned non-zero exit code: 1.
output:
Undefined symbols for architecture arm64:
  "_ZXing_Barcode_bytes", referenced from:
      _zxingcpp_cinterop_ZXing_Barcode_bytes_wrapper21 in test.kexe.o
...
ld: symbol(s) not found for architecture x86_64
FAILURE: Build failed with an exception.
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
