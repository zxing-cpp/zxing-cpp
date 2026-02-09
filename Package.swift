// swift-tools-version:5.7.1
import PackageDescription

let package = Package(
    name: "ZXingCpp",
    platforms: [
        .macOS(.v13), .iOS(.v12)
    ],
    products: [
        .library(
            name: "ZXingCpp",
            targets: ["ZXingCpp"])
    ],
    targets: [
        .target(
            name: "ZXingCppCore",
            path: "core/src",
            exclude: ["libzint", "ZXingC.cpp", "ZXingCpp.cpp"],
            publicHeadersPath: ".",
            cxxSettings: [
                .headerSearchPath("../../wrappers/ios/Sources/Wrapper"),
                .define("ZXING_INTERNAL")
            ]
        ),
        .target(
            name: "ZXingCpp",
            dependencies: ["ZXingCppCore"],
            path: "wrappers/ios/Sources/Wrapper",
            publicHeadersPath: ".",
            linkerSettings: [
                .linkedFramework("CoreGraphics"),
                .linkedFramework("CoreImage"),
                .linkedFramework("CoreVideo")
            ]
        )
    ],
    cxxLanguageStandard: CXXLanguageStandard.cxx20
)
