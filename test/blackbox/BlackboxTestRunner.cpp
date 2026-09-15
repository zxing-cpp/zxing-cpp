// Copyright 2026 Axel Waggershauser
// SPDX-License-Identifier: Apache-2.0

#include "BlackboxTestRunner.h"

#include "ByteArray.h"
#include "ImageLoader.h"
#include "ReadBarcode.h"
#include "StdPrint.h"
#include "Utf.h"
#include "Version.h"
#include "ZXAlgorithms.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <format>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#ifndef PRINT_DEBUG
#include <future>
#endif

namespace ZXing::Test {

static int failed = 0;
static int extra = 0;
static int totalImageLoadTime = 0;

static std::string Abbrev(std::string_view str, size_t maxLength)
{
	return str.size() <= maxLength ? std::string(str) : std::string(str.substr(0, maxLength)) + "...";
}

static bool isImage(const fs::path& path)
{
	return Contains({".webp", ".png", ".jpg", ".pgm", ".gif"}, path.extension());
}

static int timeSince(std::chrono::steady_clock::time_point startTime)
{
	auto duration = std::chrono::steady_clock::now() - startTime;
	return narrow_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
}

// pre-load images into cache, so the disc io time does not end up in the timing measurement
static void preloadImageCache(const std::vector<fs::path>& imgPaths)
{
	auto startTime = std::chrono::steady_clock::now();
	ImageLoader::clearCache();
	for (const auto& imgPath : imgPaths)
		ImageLoader::load(imgPath);
	totalImageLoadTime += timeSince(startTime);
}

static std::vector<fs::path> getImagesInDirectory(const fs::path& directory)
{
	std::vector<fs::path> result;
	for (const auto& entry : fs::directory_iterator(directory))
		if (fs::is_regular_file(entry.status()) && Contains({".webp", ".png", ".jpg", ".pgm", ".gif"}, entry.path().extension()))
			result.push_back(entry.path());

	preloadImageCache(result);

	return result;
}

enum class TestMode { Slow, Fast, Pure };

static std::string_view ToString(TestMode mode)
{
	switch (mode) {
	case TestMode::Slow: return "slow";
	case TestMode::Fast: return "fast";
	case TestMode::Pure: return "pure";
	}
	return {};
}

struct TestFilter
{
	bool modes[3 * 4] = {}; // slow, fast, pure x 0, 90, 180, 270 degrees

	bool& operator()(TestMode mode, int rotation)
	{
		if (static_cast<int>(mode) < 0 || static_cast<int>(mode) >= 3 || rotation < 0 || rotation > 3)
			throw std::out_of_range("Invalid mode or rotation");
		return modes[static_cast<int>(mode) * 4 + rotation];
	}
	bool operator()(TestMode mode, int rotation) const { return const_cast<TestFilter*>(this)->operator()(mode, rotation); }

	void operator|=(const TestFilter& other) { std::ranges::transform(modes, other.modes, modes, std::logical_or<>{}); }
	void operator&=(const TestFilter& other) { std::ranges::transform(modes, other.modes, modes, std::logical_and<>{}); }

	TestFilter operator!() const
	{
		TestFilter res;
		std::ranges::transform(modes, res.modes, std::logical_not<>{});
		return res;
	}
};

static TestFilter parseTestFilter(std::string_view value)
{
	TestFilter result;
	TestMode mode = static_cast<TestMode>(-1);

	for (char c : value) {
		switch (c) {
		case ' ': break;
		case 's': mode = TestMode::Slow; break;
		case 'f': mode = TestMode::Fast; break;
		case 'p': mode = TestMode::Pure; break;
		case '0': [[fallthrough]];
		case '1': [[fallthrough]];
		case '2': [[fallthrough]];
		case '3': result(mode, c - '0') = true; break;
		case 'h': result(mode, 0) = result(mode, 2) = true; break;
		case 'v': result(mode, 1) = result(mode, 3) = true; break;
		case 'a':
			for (int r : {0, 1, 2, 3})
				result(mode, r) = true;
			break;
		default: throw std::invalid_argument(std::format("Invalid search/find char '{}' in '{}'", c, value));
		}
	}
	return result;
}

using Properties = std::map<std::string, std::string>;

static std::optional<std::string> readFile(const fs::path& path)
{
	std::ifstream ifs(path, std::ios::binary);
	return ifs ? std::optional(std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>())) : std::nullopt;
}

static std::string parseTomlValue(std::string_view value, const fs::path& path, std::string_view key)
{
	auto hexDigit = [](char c) {
		if (c >= '0' && c <= '9')
			return c - '0';
		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return -1;
	};

	value = TrimWS(value);
	if (value.size() < 2 || value.front() != '"' || value.back() != '"')
		return std::string(value);

	std::string result;
	for (size_t i = 1; i + 1 < value.size(); ++i) {
		char c = value[i];
		if (c != '\\') {
			result.push_back(c);
			continue;
		}

		auto parseHexCodepoint = [&](int digitCount) {
			if (i + digitCount >= value.size())
				throw std::invalid_argument(std::format("{}: invalid unicode escape in '{}'", path.string(), key));
			uint32_t codepoint = 0;
			for (int j = 0; j < digitCount; ++j) {
				int digit = hexDigit(value[++i]);
				if (digit < 0)
					throw std::invalid_argument(std::format("{}: invalid unicode escape in '{}'", path.string(), key));
				codepoint = (codepoint << 4) | static_cast<uint32_t>(digit);
			}
			return codepoint;
		};

		if (i + 1 >= value.size() - 1)
			throw std::invalid_argument(std::format("{}: invalid escape in '{}'", path.string(), key));
		char next = value[++i];
		switch (next) {
		case '"': result.push_back('"'); break;
		case '\\': result.push_back('\\'); break;
		case 'b': result.push_back('\b'); break;
		case 't': result.push_back('\t'); break;
		case 'n': result.push_back('\n'); break;
		case 'f': result.push_back('\f'); break;
		case 'r': result.push_back('\r'); break;
		case 'u': AppendToUtf8(result, parseHexCodepoint(4)); break;
		case 'U': AppendToUtf8(result, parseHexCodepoint(8)); break;
		default: throw std::invalid_argument(std::format("{}: unsupported escape in '{}'", path.string(), key));
		}
	}
	return result;
}

static std::vector<Properties> readToml(const fs::path& path, const Properties& folderDefaults = {})
{
	auto content = readFile(path);
	if (!content)
		return {folderDefaults};

	std::vector<Properties> records;
	auto defaults = folderDefaults;
	auto current = defaults;
	bool atTopLevel = true;
	std::stringstream lines(*content);
	std::string rawLine;
	while (std::getline(lines, rawLine)) {
		auto line = TrimWS(rawLine);
		if (line.empty() || line.front() == '#')
			continue;
		if (line == "[[symbol]]") {
			if (std::exchange(atTopLevel, false))
				defaults = current; // top-level key/value pairs are defaults for all subsequent records
			else
				records.push_back(std::exchange(current, defaults));
			continue;
		}
		if (line.front() == '[')
			throw std::invalid_argument(std::format("{}: only [[symbol]] tables are supported", path.string()));
		auto equals = line.find('=');
		if (equals == std::string_view::npos)
			throw std::invalid_argument(std::format("{}: invalid TOML line '{}'", path.string(), line));
		auto key = TrimWS(line.substr(0, equals));
		auto value = parseTomlValue(line.substr(equals + 1), path, key);
		if (key.empty())
			throw std::invalid_argument(std::format("{}: invalid TOML line '{}'", path.string(), line));
		// std::println("Parsed TOML line '{}': '{}' = '{}'", rawLine, key, value);
		current[std::string(key)] = std::move(value);
	}
	records.push_back(std::move(current));
	return records;
}

static std::vector<Properties> readTestConfig(const fs::path& imagePath, const Properties& defaults)
{
	auto dataPath = imagePath;
	while (!fs::exists(dataPath.replace_extension(".toml")) && !fs::exists(dataPath.replace_extension(".txt"))) {
		auto stem = dataPath.stem().string();
		auto sep = stem.find_last_of("-_!");
		if (sep == std::string::npos)
			break;
		dataPath.replace_filename(stem.substr(0, sep));
	}

	std::vector<Properties> ret = {defaults};
	if (dataPath.extension() == ".toml")
		ret = readToml(dataPath, defaults);
	else if (dataPath.extension() == ".txt") {
		if (auto text = readFile(dataPath))
			ret.front().insert_or_assign("TextEscaped", EscapeNonGraphical(*text));
	}

	auto stem = imagePath.stem().string();
	if (stem.ends_with("!") || !fs::exists(dataPath)) {
		ret.front().insert_or_assign("missing", "sa fa pa");
	} else if (stem.ends_with("!f")) {
		ret.front().insert_or_assign("missing", "fa pa");
	} else if (stem.ends_with("!p")) {
		ret.front().insert_or_assign("missing", "pa");
	}

	// std::println("Test config for {} = {}: {}", imagePath.string(), dataPath.string(), ret.front().size());

	return ret;
}

struct Test
{
	fs::path imgPath;
	std::vector<Properties> props;
	ReaderOptions opts;
	TestFilter search;
	std::array<std::vector<Barcode>, 4> found;

	std::optional<std::string> prop(const std::string& key, int idx = 0) const
	{
		auto it = props[idx].find(key);
		return it != props[idx].end() ? std::optional<std::string>{it->second} : std::nullopt;
	}

	Test(fs::path imgPath, const Properties& defaults)
		: imgPath(std::move(imgPath)), props(readTestConfig(this->imgPath, defaults))
	{
		if (auto val = prop("formats"))
			opts.formats(BarcodeFormats(*val));
		if (auto val = prop("eanAddOnSymbol"))
			opts.eanAddOnSymbol(*val == "require" ? EanAddOnSymbol::Require
								: *val == "read"  ? EanAddOnSymbol::Read
												  : EanAddOnSymbol::Ignore);
		if (auto val = prop("maxNumberOfSymbols"))
			opts.maxNumberOfSymbols(std::stoi(*val));
		if (auto val = prop("returnErrors"))
			opts.returnErrors(*val == "true");

		search = parseTestFilter(*prop("find"));
		if (auto val = prop("search"))
			search |= parseTestFilter(*val);
	}

	void run(TestMode mode)
	{
		auto opts = this->opts;
		opts.tryDownscale(false).downscaleFactor(2).downscaleThreshold(180);
		opts.tryHarder(mode == TestMode::Slow);
		opts.tryRotate(mode == TestMode::Slow);
		opts.tryInvert(mode == TestMode::Slow);
		opts.isPure(mode == TestMode::Pure);
		if (mode == TestMode::Pure)
			opts.binarizer(Binarizer::FixedThreshold);
		// opts.maxNumberOfSymbols(1);

		for (int rotation : {0, 1, 2, 3}) {
			if (search(mode, rotation))
				found[rotation] = ReadBarcodes(ImageLoader::load(imgPath).rotated(rotation * 90), opts);
			else
				found[rotation].clear();
			// std::println("    {} @ {}/{:3} => found {} barcodes", imgPath.filename().string(), ToString(mode), rotation * 90,
			// Size(found[rotation]));
		}
	}
};

static void runBlackBoxTestDirectory(const fs::path& directory)
{
	auto stem = directory.stem().string();
	if (auto pos = stem.find_first_of('-'); pos != std::string::npos)
		stem = stem.substr(0, pos);
	auto format = BarcodeFormat::None;
	try {
		format = BarcodeFormatFromString(stem);
	} catch (...) {
	}

	Properties defaults;
	defaults["find"] = format & BarcodeFormat::AllLinear ? "sh fh" : "sa fh";
	if (format != BarcodeFormat::None) {
		defaults["Format"] = ToString(format);
	}

	defaults = readToml(directory / "!defaults.toml", defaults).front();
	auto imagePaths = getImagesInDirectory(directory);
	std::vector<Test> tests;
	for (const auto& imagePath : imagePaths)
		tests.emplace_back(imagePath, defaults);

	std::map<fs::path, std::map<std::string, TestFilter>> missing, unexpected;

	std::string modeSummaries;
	for (auto mode : {TestMode::Slow, TestMode::Fast, TestMode::Pure}) {
		auto startTime = std::chrono::steady_clock::now();
#ifndef PRINT_DEBUG
		auto futures = std::vector<std::future<void>>{};
		for (auto& test : tests)
			futures.push_back(std::async(std::launch::async, [&] { test.run(mode); }));
		for (auto& f : futures)
			f.wait();
#else
		for (auto& test : tests)
			test.run(mode);
#endif
		int decodeTime = timeSince(startTime);
		std::array<int, 4> numFound{};

		for (auto& test : tests) {
			for (int rotation : {0, 1, 2, 3}) {
				numFound.at(rotation) += Size(test.found.at(rotation));
				auto found = test.found.at(rotation);

				if (auto val = test.prop("SequenceIndex", Size(test.props) - 1); val == "-1" && !found.empty())
					found.push_back(MergeStructuredAppendSequence(found));

				for (const auto& expected : test.props) {
					auto expectAt = parseTestFilter(expected.at("find"));
					if (expected.contains("missing"))
						expectAt &= !parseTestFilter(expected.at("missing"));
					if (!expectAt(mode, rotation))
						continue;

					// search expected in found, if not in found, report missing
					auto it = std::ranges::find_if(found, [&](const auto& barcode) {
						for (const auto& [key, value] : expected) {
							if (IsUpper(key.front()) && barcode.extra(key) != value) {
								// println("    Mismatch for key '{}': expected '{}' but got '{}'", key, value,
								// barcode.extra(key));
								return false;
							}
						}
						return true;
					});
					if (it == found.end()) {
						std::string str;
						for (const auto& [key, value] : expected)
							str += std::format(" {}=\"{}\"", key, Abbrev(EscapeNonGraphical(value), 30));
						missing[test.imgPath][str](mode, rotation) = true;
						failed++;
					} else {
						found.erase(it);
					}
				}

				for (const auto& barcode : found) {
					auto str = std::format("{}: \"{}\"", ToString(barcode.format()), Abbrev(barcode.text(TextMode::Escaped), 30));
					unexpected[test.imgPath][str](mode, rotation) = true;
				}
			}
		}

		modeSummaries += std::format("| {}: {:3} {:3} {:3} {:3}  -> {:3}ms ", ToString(mode), numFound[0], numFound[1],
									 numFound[2], numFound[3], decodeTime);
	}
	std::println("{:25} {:3} images {}", directory.stem().string(), Size(imagePaths), modeSummaries);

	auto printMismatches = [&](const std::map<fs::path, std::map<std::string, TestFilter>>& mismatches, std::string_view title) {
		for (const auto& [imagePath, expected] : mismatches) {
			for (const auto& [expectedText, found] : expected) {
				std::print("{:35} ", imagePath.lexically_relative(directory.parent_path()).string());
				for (const auto mode : {TestMode::Slow, TestMode::Fast, TestMode::Pure}) {
					std::print(" | {}:", ToString(mode));
					for (int rotation : {0, 1, 2, 3})
						std::print(" {:>3}", found(mode, rotation) ? 'X' : ' ');
					std::print("          ");
				}
				std::println(" | {}: {}", title, expectedText);
			}
		}
	};
	printMismatches(missing, "missing");
	printMismatches(unexpected, "unexpected");

	extra += Reduce(unexpected, 0, [](int sum, const auto& p) { return sum + Size(p.second); });
}

int runBlackBoxTests(const fs::path& testPathPrefix, const std::set<std::string>& includedTests)
{
	std::vector<fs::path> directories;
	for (const auto& entry : fs::directory_iterator(testPathPrefix))
		if (fs::is_directory(entry) && (includedTests.empty() || std::ranges::any_of(includedTests, [&](const auto& prefix) {
											return entry.path().stem().string().starts_with(prefix);
										})))
			directories.push_back(entry.path());
	std::ranges::sort(directories);

	auto startTime = std::chrono::steady_clock::now();
	for (const auto& directory : directories)
		runBlackBoxTestDirectory(directory);

	int totalTime = timeSince(startTime);
	int decodeTime = totalTime - totalImageLoadTime;
	std::println("load time:   {} ms.", totalImageLoadTime);
	std::println("decode time: {} ms.", decodeTime);
	std::println("total time:  {} ms.", totalTime);
	if (failed)
		std::println("WARNING: {} tests failed.", failed);
	if (extra)
		std::println("INFO: {} unexpected symbols found.", extra);

	return failed;
}

} // namespace ZXing::Test
