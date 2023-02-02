echo ========= Remove previous builds
rm -rf _builds
rm -rf ZXingCpp.xcframework

echo ========= Create project structure
cmake -S../../ -B_builds -GXcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_INSTALL_PREFIX=`pwd`/_install \
    -DCMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH=NO \
    -DBUILD_UNIT_TESTS=NO \
    -DBUILD_BLACKBOX_TESTS=NO \
    -DBUILD_EXAMPLES=NO \
    -DBUILD_APPLE_FRAMEWORK=YES

echo ========= Build the sdk for simulators
xcodebuild -project _builds/ZXing.xcodeproj build \
    -target ZXing \
    -parallelizeTargets \
    -configuration Release \
    -hideShellScriptEnvironment \
    -sdk iphonesimulator

echo ========= Build the sdk for iOS
xcodebuild -project _builds/ZXing.xcodeproj build \
    -target ZXing \
    -parallelizeTargets \
    -configuration Release \
    -hideShellScriptEnvironment \
    -sdk iphoneos

echo ========= Create the xcframework
xcodebuild -create-xcframework \
    -framework ./_builds/core/Release-iphonesimulator/ZXing.framework \
    -framework ./_builds/core/Release-iphoneos/ZXing.framework \
    -output ZXingCpp.xcframework
