/*
 * Copyright 2023 Antoine Mérino
 */
// SPDX-License-Identifier: Apache-2.0

#include "DecodeHints.h"
#include "GTIN.h"
#include "ODDXFilmEdgeReader.h"
#include "Result.h"
#include "ZXAlgorithms.h"

#include <optional>
#include <set>
namespace ZXing::OneD {

// Detection is made from center to bottom.
// We ensure the clock signal is decoded before the data signal to avoid false positives.
// They are two version of a DX Edge codes : without half-frame information and with half-frame information.
// The clock signal is longer if the DX code contains the half-frame information (more recent version)
constexpr int CLOCK_PATTERN_LENGTH_HF = 31;
constexpr int CLOCK_PATTERN_LENGTH_NO_HF = 23;
constexpr int DATA_START_PATTERN_SIZE = 5;
constexpr auto CLOCK_PATTERN_COMMON = FixedPattern<15, 20> {5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
constexpr auto CLOCK_PATTERN_HF =
	FixedPattern<25, CLOCK_PATTERN_LENGTH_HF>{5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3};
constexpr auto CLOCK_PATTERN_NO_HF = FixedPattern<17, CLOCK_PATTERN_LENGTH_NO_HF>{5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3};
constexpr auto DATA_START_PATTERN_ = FixedPattern<5, 5>    {1, 1, 1, 1, 1};
constexpr auto DATA_STOP_PATTERN_ = FixedPattern<3, 3>     {1, 1, 1};

// Signal data length, without the start and stop patterns
constexpr int DATA_LENGTH_HF = 23;
constexpr int DATA_LENGTH_NO_HF = 15;


/**
 * @brief Parse a part of a vector of bits (boolean) to a decimal number.
 * Eg: {1, 1, 0} -> 6.
 * @param begin begin of the vector's part to be parsed
 * @param end end of the vector's part to be parsed
 * @return The decimal value of the parsed part
 */
int toDecimal(const std::vector<bool>::iterator begin, const std::vector<bool>::iterator end)
{
	int retval = 0;
	auto i = std::distance(begin, end) - 1;
	for (std::vector<bool>::iterator it = begin; it != end; it++, i--) {
		retval += (*it ? (1 << i) : 0);
	}
	return retval;
}

// DX Film Edge Clock signal found on 35mm films.
struct Clock {
	int rowNumber = 0;
	bool containsHFNumber = false; // Clock signal (thus data signal) with half-frame number (longer version)
	int xStart = 0; // Beginning of the clock signal on the X-axis, in pixels
	int xStop = 0; // End of the clock signal on the X-axis, in pixels
	int pixelTolerance = 0; // Pixel tolerance will be set depending of the length of the clock signal (in pixels)

	bool operator<(const int x) const
	{
		return xStart < x;
	}

	bool operator < (const Clock& other) const
	{
		return xStart < other.xStart;
	}

	/*
	* @brief Check if this clocks start at about the same x position as another.
	* We assume two clock are the same when they start in about the same X position,
	* even if they are different clocks (stop at different position or different type).
	* Only the more recent clock is kept.
	*/
	bool xStartInRange(const Clock& other) const
	{
		auto tolerance = std::max(pixelTolerance, other.pixelTolerance);
		return (xStart - tolerance) <= other.xStart && (xStart + tolerance) >= other.xStart;
	}

	bool xStartInRange(const int x) const
	{
		return (xStart - pixelTolerance) <= x && (xStart + pixelTolerance) >= x;
	}

	bool xStopInRange(const int x) const
	{
		return (xStop - pixelTolerance) <= x && (xStop + pixelTolerance) >= x;
	}

	/*
	* @brief Check the clock's row number is next to the row we want to compare.
	* Since we update the clock row number with the latest found signal's row number,
	* the signal may be either:
	* - below the clock, but not too far
	* - slightly above the clock
	* @param otherRownumber the other row to check if it's in range or not
	* @param totalRows the image total row number (~image height)
	*/
	bool rowInRange(const int otherRowNumber, const int totalRows) const
	{
		const auto acceptedRowRange = totalRows / 5 + 1; // Below the clock, not too far
		const auto rowMarginTolerance = totalRows / 20 + 1; // If above the clock, it should be really close
		auto result = ((otherRowNumber >= rowNumber && otherRowNumber - rowNumber <= acceptedRowRange)
					   || (rowNumber <= otherRowNumber && otherRowNumber - rowNumber <= rowMarginTolerance));
		return result;
	}

};

class ClockSet : public std::set<Clock, std::less<>> {
public:

	/*
	* @brief Return the clock which starts at the closest X position.
	*/
	const ClockSet::iterator closestElement(const int x)
	{
		const auto it = lower_bound(x);
		if (it == begin())
			return it;

		const auto prev_it = std::prev(it);
		return (it == end() || x - prev_it->xStart <= it->xStart - x) ? prev_it : it;
	}

	/**
	 * @brief Add a new clock to the set.
	 * If the new clock is close enough to an existing clock in the set,
	 * the old clock is removed.
	 */
	void update(const Clock& newClock)
	{
		auto closestClock = closestElement(newClock.xStart);
		if (closestClock != end() && newClock.xStartInRange(*closestClock)) {
			erase(closestClock);
			insert(newClock);
		} else {
			insert(newClock);
		}
	}
};


/*
* @brief To avoid many false positives,
* the clock signal must be found to attempt to decode a data signal.
* We ensure the data signal starts below a clock.
* We accept a tolerance margin,
* ie. the signal may start a few pixels before or after the clock on the X-axis.
*/
struct DXFEState : public RowReader::DecodingState {
	ClockSet allClocks;
	int totalRows = 0;
};

/**
 * @brief Try to find a DX Film Edge clock in the given row.
 * @param rowNumber the row number
 * @param end end of the vector's part to be parsed
 * @return The decimal value of the parsed part
 */
std::optional<Clock> findClock(int rowNumber, PatternView& view)
{
	// Minimum "allowed "white" zone to the left and the right sides of the clock signal.
	constexpr float minClockNoHFQuietZone = 2;
	// On HF versions, the decimal number uses to be really close to the clock
	constexpr float minClockHFQuietZone = 0.5; 

	// Adjust the pixel shift tolerance between the data signal and the clock signal.
	// 1 means the signal can be shifted up to one bar to the left or the right.
	constexpr float pixelToleranceRatio = 0.5;

	// Before detecting any clock,
	// try to detect the common pattern between all types of clocks.
	// This avoid doing two detections at each interations instead of one,
	// when they is no DX Edge code to detect.
	auto commonClockPattern =
		FindLeftGuard(view, CLOCK_PATTERN_COMMON.size(), CLOCK_PATTERN_COMMON, std::min(minClockNoHFQuietZone, minClockHFQuietZone));
	if (commonClockPattern.isValid()) {
		bool foundClock = false;
		bool containsHFNumber = false;
		auto clock_pattern = FindLeftGuard(view, CLOCK_PATTERN_HF.size(), CLOCK_PATTERN_HF, minClockHFQuietZone);
		if (clock_pattern.isValid()) {
			foundClock = true;
			containsHFNumber = true;
		} else {
			clock_pattern = FindLeftGuard(view, CLOCK_PATTERN_NO_HF.size(), CLOCK_PATTERN_NO_HF, minClockNoHFQuietZone);
			if (clock_pattern.isValid())
				foundClock = true;
		}
		if (foundClock) {
			Clock clock;
			clock.rowNumber = rowNumber;
			clock.containsHFNumber = containsHFNumber;
			clock.xStart = clock_pattern.pixelsInFront();
			clock.xStop = clock_pattern.pixelsTillEnd();
			clock.pixelTolerance = (clock_pattern.pixelsTillEnd() - clock_pattern.pixelsInFront())
								   / (containsHFNumber ? CLOCK_PATTERN_LENGTH_HF : CLOCK_PATTERN_LENGTH_NO_HF) * pixelToleranceRatio;
			return clock;
		}
	}
	return std::nullopt;
}


Result DXFilmEdgeReader::decodePattern(int rowNumber, PatternView& next, std::unique_ptr<DecodingState>& state) const
{

	// Retrieve the decoding state to check if a clock signal has already been found before.
	// We check also if it contains the half-frame number, since it affects the signal structure.
	if (!state) {
		state.reset(new DXFEState);
		// We need the total row number to adjust clock & signal detection sensitivity
		static_cast<DXFEState*>(state.get())->totalRows = 2 * rowNumber;
	}
	auto& allClocks = static_cast<DXFEState*>(state.get())->allClocks;
	auto& totalRows = static_cast<DXFEState*>(state.get())->totalRows;
	// Minimum "allowed "white" zone to the left and the right sides of the data signal.
	// We allow a smaller quiet zone, ie improve detection at risk of getting false positives,
	// because the risk is greatly reduced when we check we found the clock before the signal.
	constexpr float minDataQuietZone = 0.2;

	// We should find at least one clock before attempting to decode the data signal.
	auto clock = findClock(rowNumber, next);

	if (clock)
		allClocks.update(clock.value());

	if (allClocks.empty())
		return {};

	// Now that we found at least a clock, attempt to decode the data signal.
	// Start by finding the data start pattern.
	next = FindLeftGuard(next, DATA_START_PATTERN_.size(), DATA_START_PATTERN_, minDataQuietZone);
	if (!next.isValid())
		return {};

	auto xStart = next.pixelsInFront();

	// The found data signal must be below the clock signal, otherwise we abort the decoding (potential false positive)
	auto closestClock = allClocks.closestElement(xStart);
	if (!closestClock->xStartInRange(xStart))
		return {};

	// Avoid decoding a signal found at the top or too far from the clock
	// (might happen when stacking two films one of top of the other, or other false positive situations)
	if (!closestClock->rowInRange(rowNumber, totalRows))
		return {};

	//  Compute the length of a bar
	// It may be greater than 1 depending on what have been found in the raw signal
	auto perBarRawWidth = *next.data();

	// Skip the data start pattern (black, white, black, white, black)
	// The first signal bar is always white: this is the
	// separation between the start pattern and the product number)
	next.shift(DATA_START_PATTERN_SIZE);

	if (!next.isValid())
		return {};

	std::vector<bool> dataSignalBits;

	// They are two possible data signal lengths (with or without half-frame information)
	dataSignalBits.reserve(closestClock->containsHFNumber ? DATA_LENGTH_HF : DATA_LENGTH_NO_HF);

	// Populate a vector of booleans to represent the bits. true = black, false = white.
	// We start the parsing just after the data start signal.
	// The first bit is always a white bar (we include the separator just after the start pattern)
	// Eg: {3, 1, 2} -> {0, 0, 0, 1, 0, 0}
	int signalLength = 0;
	bool currentBarIsBlack = false; // the current bar is white
	while (signalLength < (closestClock->containsHFNumber ? DATA_LENGTH_HF : DATA_LENGTH_NO_HF)) {
		if (!next.isValid())
			return {};

		// Zero means we can't conclude on black or white bar. Abort the decoding.
		if (*next.data() == 0)
			return {};

		// Adjust the current bar according to the computed ratio above.
		// When the raw result is not exact (between two bars),
		// we round the bar size to the nearest integer.
		auto currentBarWidth =
			*next.data() / perBarRawWidth + (*next.data() % perBarRawWidth >= (perBarRawWidth / 2) ? 1 : 0);

		signalLength += currentBarWidth;

		// Extend the bit array according to the current bar length.
		// Eg: one white bars -> {0}, three black bars -> {1, 1, 1}
		while (currentBarWidth > 0
			   && (int)dataSignalBits.size() < (closestClock->containsHFNumber ? DATA_LENGTH_HF : DATA_LENGTH_NO_HF)) {
			dataSignalBits.push_back(currentBarIsBlack);
			--currentBarWidth;
		}

		// Iterate to the next data signal bar (the color is inverted)
		currentBarIsBlack = !currentBarIsBlack;
		next.shift(1);
	}

	// Check the signal length
	if (signalLength != (closestClock->containsHFNumber ? DATA_LENGTH_HF : DATA_LENGTH_NO_HF))
		return {};
	
	// Check there is the Stop pattern at the end of the data signal
	next = next.subView(0, 3);
	if (!IsRightGuard(next, DATA_STOP_PATTERN_, minDataQuietZone))
		return {};

	// Check the data signal has been fully parsed
	if (closestClock->containsHFNumber && dataSignalBits.size() < DATA_LENGTH_HF)
		return {};
	if (!closestClock->containsHFNumber && dataSignalBits.size() < DATA_LENGTH_NO_HF)
		return {};

	// The following bits are always white (=false), they are separators.
	if (dataSignalBits.at(0) || dataSignalBits.at(8))
		return {};
	if (closestClock->containsHFNumber && (dataSignalBits.at(20) || dataSignalBits.at(22)))
		return {};
	if (!closestClock->containsHFNumber && (dataSignalBits.at(8) || dataSignalBits.at(14)))
		return {};

	// Check the parity bit
	auto signalSum = std::accumulate(dataSignalBits.begin(), dataSignalBits.end()-2, 0);
	auto parityBit = *(dataSignalBits.end() - 2);
	if (signalSum % 2 != (int)parityBit)
		return {};

	// Compute the DX 1 number (product number)
	auto productNumber = toDecimal(dataSignalBits.begin() + 1, dataSignalBits.begin() + 8);
	if (!productNumber)
		return {};

	// Compute the DX 2 number (generation number)
	auto generationNumber = toDecimal(dataSignalBits.begin() + 9, dataSignalBits.begin() + 13);

	// Generate the textual representation.
	// Eg: 115-10/11A means: DX1 = 115, DX2 = 10, Frame number = 11A
	std::string txt;
	txt.reserve(10);
	txt = std::to_string(productNumber) + "-" + std::to_string(generationNumber);
	if (closestClock->containsHFNumber) {
		auto halfFrameNumber = toDecimal(dataSignalBits.begin() + 13, dataSignalBits.begin() + 20);
		txt += "/" + std::to_string(halfFrameNumber / 2);
		if (halfFrameNumber % 2)
			txt += "A";
	}

	Error error;

	// TODO is it required?
	// AFAIK The DX Edge barcode doesn't follow any symbology identifier.
	SymbologyIdentifier symbologyIdentifier = {'I', '0'}; // No check character validation ?

	auto xStop = next.pixelsTillEnd();

	// The found data signal must be below the clock signal, otherwise we abort the decoding (potential false positive)
	if (!closestClock->xStopInRange(xStop))
		return {};


	// Update the clock X coordinates with the latest corresponding data signal
	// This may improve signal detection for next row iterations
	if (closestClock->xStop != xStop || closestClock->xStart != xStart) {
		Clock clock(*closestClock);
		clock.xStart = xStart;
		clock.xStop = xStop;
		clock.rowNumber = rowNumber;
		allClocks.erase(closestClock);
		allClocks.insert(clock);
	}
	closestClock = allClocks.closestElement(xStart);
	return Result(txt, rowNumber, xStart, xStop, BarcodeFormat::DXFilmEdge, symbologyIdentifier, error);
}

} // namespace ZXing::OneD
