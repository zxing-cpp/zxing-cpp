# ZXing-C++ Android Library

To use the Android (wrapper) library in other apps, it is easiest
to build the library project and include the resulting AAR (Android
Archive) file in your app.

## Build

1. Install AndroidStudio including NDK and CMake (see 'SDK Tools').
2. Open the project in folder containing this README.
3. The project contains 2 modules: `zxingcpp` is the wrapper library, `app` is the demo app using `zxingcpp`.

To build the AAR (Android Archive) from the command line:

	$ ./gradlew :zxingcpp:assembleRelease

Then copy `zxingcpp/build/outputs/aar/zxingcpp-release.aar` into `app/libs` of your app.

## Publish to GitHub Packages

1. Configure `github.properties` file by passing your GitHub userId and token with read and write permissions. 
2. If you don't want to use `github.properties` use system env variables. Set GPR_USER and GPR_API_KEY
3. Execute `$ gradle publish` from command line

