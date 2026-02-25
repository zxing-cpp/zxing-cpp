/*
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "BarcodeFormat.h"

#include <version>
#ifdef __cpp_lib_format

#include "StdPrint.h"

#include <cctype>
#include <cstring>
#include <format>
#include <functional>
#include <string>

static void PrintUsage(const char* exePath)
{
	std::print("ZXingBarcodeFormat - A command line tool to generate wrapper source code for the BarcodeFormat enum\n\n");
	std::println("Usage: {} <C#|Go|K/N|Rust|Swift>", exePath);
}

static void PrintBFs(
	std::format_string<std::string, int> fmt,
	std::function<std::string(const char*)> name = [](const char* name) { return std::string(name); })
{
	std::println(fmt, name("Invalid"), 0xffff);
#define X(NAME, SYM, VAR, FLAGS, ZINT, ENABLED, HRI) std::println(fmt, name(#NAME), ZX_BCF_ID(SYM, VAR));
	ZX_BCF_LIST(X)
#undef X
}

int main(int argc, char* argv[])
{
	auto is = [&, i=1](const char* str) { return strncmp(argv[i], str, strlen(argv[i])) == 0; };

	if (argc != 2) {
		PrintUsage(argv[0]);
		return -1;
	} else if (is("Go")) {
		std::println("package zxingcpp\n");
		std::println("const (");
		PrintBFs("	BarcodeFormat{:15} BarcodeFormat = 0x{:04X}");
		std::println(")");
	} else if (is("C#")) {
		PrintBFs("	public static readonly BarcodeFormat {:15} = new BarcodeFormat(0x{:04X});");
	} else if (is("K/N")) {
		PrintBFs("	{0:15}(ZXing_BarcodeFormat.ZXing_BarcodeFormat_{0}),");
	} else if (is("Rust")) {
		PrintBFs("pub const ZXing_BarcodeFormat_{}: ZXing_BarcodeFormat = 0x{:04X};");
	} else if (is("Swift")) {
		auto swiftName = [](const char* name) {
			std::string sv(name);
			std::string ret = (char)std::tolower(sv[0]) + sv.substr(1);
			// Convert to camelCase, but keep uppercase letters if they are followed by a lowercase letter
			// (e.g. QRCode -> qrCode). This is a bit hacky, but it works for the current set of names and avoids the
			// need for a manual mapping. This ensures that the generated Swift code follows typical naming patterns.
			for (int i = 2; i < ret.size() && (std::isupper(ret[i]) || std::isdigit(ret[i])); ++i)
				ret[i - 1] = std::tolower(ret[i - 1]);
			ret.back() = std::tolower(ret.back());
			return ret == "eanupc" ? "eanUPC" : ret == "upca" ? "upcA" : ret == "upce" ? "upcE" : ret;
		};
		PrintBFs("	public static let {:15} = BarcodeFormat(rawValue: 0x{:04X})", swiftName);
	} else {
		PrintUsage(argv[0]);
		return -1;
	}

	return 0;
}

#else

#include <cstdio>

int main()
{
	printf("This tool requires C++20 support with std::format. Please use a compatible compiler.\n");
	return -1;
}

#endif
