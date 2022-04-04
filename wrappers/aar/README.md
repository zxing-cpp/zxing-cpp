# ZXing-C++ Android Archive

## How to include

### JitPack

Add the JitPack repository in your root `build.gradle` at the end of
repositories:

	allprojects {
		repositories {
			...
			maven { url 'https://jitpack.io' }
		}
	}

Then add the dependency in your `app/build.gradle`:

	dependencies {
		implementation 'com.github.nu-book:zxing-cpp:v1.2.1'
	}

### Build and use the AAR file in your app

Alternatively you can build the AAR (Android Archive) yourself:

	$ ./gradlew :zxingcpp:assembleRelease

And put `zxingcpp/build/outputs/aar/zxingcpp-release.aar` into `app/libs`
of your app.

## How to decode barcodes

Have a look at [MainActivity.kt](app/src/main/java/com/example/zxingcppdemo/MainActivity.kt)
of the sample app for details.

### Legacy Camera API

Use [ZXingCpp.readByteArray](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L84)
with the byte array from [onPreviewFrame][onPreviewFrame].
You can calculate the row stride with this [formula][rowStride].

### Camera2/CameraX API

Use [ZXingCpp.readYBuffer](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L54)
with the Y plane buffer (at index 0) from the [Image][image] object.

### Bitmap

For static images you can use [ZXingCpp.readBitmap](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L114).

## How to encode barcodes

To encode barcodes use
* [ZXingCpp.encodeAsBitmap](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L149)
* [ZXingCpp.encodeAsSvg](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L184)
* [ZXingCpp.encodeAsText](zxingcpp/src/main/java/com/nubook/android/zxingcpp/ZxingCpp.kt#L216)

[onPreviewFrame]: https://developer.android.com/reference/android/hardware/Camera.PreviewCallback#onPreviewFrame(byte[],%20android.hardware.Camera)
[rowStride]: https://developer.android.com/reference/android/hardware/Camera.Parameters#setPreviewFormat(int)
[image]: https://developer.android.com/reference/android/media/Image
