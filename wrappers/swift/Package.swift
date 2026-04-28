// swift-tools-version: 5.9

import PackageDescription

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
	targets: [
		.target(
			name: "ZXingCBridge",
			linkerSettings: [.linkedLibrary("ZXing")]
		),
		.target(
			name: "ZXingCpp",
			dependencies: ["ZXingCBridge"]
		),
		.executableTarget(
			name: "DemoReader",
			dependencies: ["ZXingCpp"],
			linkerSettings: [.linkedLibrary("c++")]
		),
		.executableTarget(
			name: "DemoWriter",
			dependencies: ["ZXingCpp"],
			linkerSettings: [.linkedLibrary("c++")]
		),
	]
)
