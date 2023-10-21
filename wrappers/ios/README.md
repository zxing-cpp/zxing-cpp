# ZXingCpp iOS Framework

## Usage

For general usage, please compare the source code provided in the demo project.

### Swift PM

As this repository provides a `Package.swift` on root level, you can add `zxing-cpp` including wrapper code by adding a Package Dependency in Xcode.

An alternative way is to check this repository out and add it as a local Swift Package by adding it as dependency to your app.

### CocoaPods

As an alternative way, you can also rely on [CocoaPods](https://cocoapods.org/pods/zxing-cpp). Just add the Pod as dependency, for instance:

```
pod 'zxing-cpp'
```

If you just need the core without the wrapper code, you can also rely on:

```
pod 'zxing-cpp/Core'
```

The module to be imported is named `ZXingCpp`.
