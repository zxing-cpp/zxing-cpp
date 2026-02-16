// swift-tools-version: 5.9

import Foundation
import PackageDescription

let env = ProcessInfo.processInfo.environment
let useBundledCore = ["1", "true", "yes", "on"].contains((env["ZXING_BUNDLED"] ?? "on").lowercased())

var targets: [Target] = []

if useBundledCore {
    targets.append(
        .target(
            name: "ZXingCore",
            path: "core/src",
            exclude: [],
            publicHeadersPath: "libzueci", // need to point to a directory with at least one header to be treated as a C target
            cxxSettings: [
                .headerSearchPath("."),
                .headerSearchPath("libzint"),
                .headerSearchPath("../../wrappers/swift/Sources/ZXingCBridge/bundled"), // Version.h
                .define("ZXING_INTERNAL")
            ]
        )
    )
}

targets += [
    .target(
        name: "ZXingCBridge",
        dependencies: [],
        path: "wrappers/swift/Sources/ZXingCBridge",
        publicHeadersPath: useBundledCore ? "bundled" : "external",
        // cSettings: useBundledCore ? [
        //     .headerSearchPath("../../../../core/src"),
        // ] : [],
        linkerSettings: useBundledCore ? [] : [.linkedLibrary("ZXing")]
    ),
    .target(
        name: "ZXingCpp",
        dependencies: useBundledCore ? ["ZXingCBridge", "ZXingCore"] : ["ZXingCBridge"],
        path: "wrappers/swift/Sources/ZXingCpp"
    ),
    .executableTarget(
        name: "DemoReader",
        dependencies: ["ZXingCpp"],
        path: "wrappers/swift/Sources/DemoReader"
    ),
    .executableTarget(
        name: "DemoWriter",
        dependencies: ["ZXingCpp"],
        path: "wrappers/swift/Sources/DemoWriter"
    ),
]

let package = Package(
    name: "ZXingCpp",
    platforms: [
        .iOS(.v13),
        .macOS(.v10_15),
    ],
    products: [
        .library(name: "ZXingCpp", targets: ["ZXingCpp"]),
        .executable(name: "demo_reader", targets: ["DemoReader"]),
        .executable(name: "demo_writer", targets: ["DemoWriter"]),
    ],
    targets: targets,
    cxxLanguageStandard: .cxx20
)
