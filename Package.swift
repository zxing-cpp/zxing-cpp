// swift-tools-version:5.7.1
import PackageDescription

let package = Package(
    name: "ZXingCppWrapper",
    platforms: [
        .iOS(.v11)
    ],
    products: [
        .library(
            name: "ZXingCppWrapper",
            targets: ["ZXingCppWrapper"])
    ],
    targets: [
        .target(
            name: "ZXingCpp",
            path: "core/src",
            publicHeadersPath: "."
        ),
        .target(
            name: "ZXingCppWrapper",
            dependencies: ["ZXingCpp"],
            path: "wrappers/ios/Sources/Wrapper",
            publicHeadersPath: ".",
            linkerSettings: [
                .linkedFramework("CoreGraphics"),
                .linkedFramework("CoreImage"),
                .linkedFramework("CoreVideo")
            ]
        )
    ],
    cxxLanguageStandard: CXXLanguageStandard.gnucxx20
)
