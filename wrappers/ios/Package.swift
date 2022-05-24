// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "ZXingWrapper",
    platforms: [
        .iOS(.v13)
    ],
    products: [
        .library(
            name: "ZXingWrapper",
            type: .static,
            targets: ["ZXingWrapper"])
    ],
    targets: [
        .binaryTarget(
            name: "ZXing",
            path: "ZXing.xcframework"
        ),
        .target(
            name: "ZXingWrapper",
            dependencies: ["ZXing"],
            path: "Sources/Wrapper",
            publicHeadersPath: ".",
            cxxSettings: [
                .unsafeFlags(["-stdlib=libc++"]),
                .unsafeFlags(["-std=gnu++17"])
            ]
        )
    ]
)
