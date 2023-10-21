# ZXingCpp iOS Framework

## Installation

### SwiftPM

As this repository provides a `Package.swift` on the root level, so you can add `zxing-cpp` including wrapper code by adding a Package Dependency in Xcode.

An alternative way is to check this repository out and add it as a local Swift Package by adding it as a dependency to your app.

### CocoaPods

You can also use [CocoaPods](https://cocoapods.org/pods/zxing-cpp). Just add the Pod as a dependency:

```
pod 'zxing-cpp'
```
The module to be imported is named `ZXingCpp`. If you just need the core without the wrapper code, you can use:
```
pod 'zxing-cpp/Core'
```

## Usage

For general usage of the ObjectiveC/Swift wrapper, please have a look at the source code provided in the [demo project](demo).
