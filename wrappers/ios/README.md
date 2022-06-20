# ZXingCpp iOS Framework

To use the iOS (wrapper) framework in other apps, it is easiest
to build the library project and include the resulting xcframework
file in your app.

## How to build and use

To build the xcframework:

	$ ./build-release.sh

Then copy `zxingcpp/wrappers/ios/ZXingCpp.xcframework` into the 
frameworks-section of your app.
