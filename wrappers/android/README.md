# ZXing-C++ Android Library

## Install

The easiest way to use the library is to fetch if from _mavenCentral_. Simply add **one** of the following two lines
```gradle
implementation("io.github.zxing-cpp:android:2.3.0")
implementation("io.github.zxing-cpp:android:2.4.0-SNAPSHOT")
```
to your `build.gradle.kts` file in the `dependencies` section. To access the SNAPSHOT version, you also need to add a separate repositories entry in your `build.cradle.kts` file:

```gradle
maven { url = uri("https://s01.oss.sonatype.org/content/repositories/snapshots") }
```

## Use

A trivial use case looks like this (in Kotlin):

```kotlin
import zxingcpp.BarcodeReader

var barcodeReader = BarcodeReader()

fun process(image: ImageProxy) {
    image.use {
        barcodeReader.read(it)
    }.joinToString("\n") { result ->
        "${result.format} (${result.contentType}): ${result.text}"
    }
}
```

## Build locally

1. Install AndroidStudio including NDK and CMake (see 'SDK Tools').
2. Open the project in folder containing this README.
3. The project contains 2 modules: `zxingcpp` is the wrapper library, `app` is the demo app using `zxingcpp`.

To build the AAR (Android Archive) from the command line:

	$ ./gradlew :zxingcpp:assembleRelease

Then copy `zxingcpp/build/outputs/aar/zxingcpp-release.aar` into `app/libs` of your app.


