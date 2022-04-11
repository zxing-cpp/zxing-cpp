# ZXing-C++ Android Library

To use the Android (wrapper) library in other apps, it is easiest
to build the library project and include the resulting AAR (Android
Archive) file in your app.

## How to build and use

To build the AAR (Android Archive):

	$ ./gradlew :zxingcpp:assembleRelease

Then copy `zxingcpp/build/outputs/aar/zxingcpp-release.aar` into
`app/libs` of your app.

Check the included sample app on how to use `com.zxingcpp.BarcodeReader`.
