/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
* Copyright 2023 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Point.h"
#include "QRECB.h"
#include "QRErrorCorrectionLevel.h"

#include <array>
#include <initializer_list>
#include <vector>

namespace ZXing {

class BitMatrix;

namespace QRCode {

/**
* See ISO 18004:2006 Annex D
*/
class Version
{
public:
	Type type() const { return _type; }
	bool isMicro() const { return type() == Type::Micro; }
	bool isRMQR() const { return type() == Type::rMQR; }
	bool isModel1() const { return type() == Type::Model1; }
	bool isModel2() const { return type() == Type::Model2; }

	int versionNumber() const { return _versionNumber; }

	const std::vector<int>& alignmentPatternCenters() const { return _alignmentPatternCenters; }

	int totalCodewords() const { return _totalCodewords; }

	int dimension() const { return DimensionOfVersion(_versionNumber, isMicro()); }

	const ECBlocks& ecBlocksForLevel(ErrorCorrectionLevel ecLevel) const { return _ecBlocks[(int)ecLevel]; }

	BitMatrix buildFunctionPattern() const;

	static constexpr int DimensionStep(bool isMicro) { return std::array{4, 2}[isMicro]; }
	static constexpr int DimensionOffset(bool isMicro) { return std::array{17, 9}[isMicro]; }
	static constexpr int DimensionOfVersion(int version, bool isMicro)
	{
		return DimensionOffset(isMicro) + DimensionStep(isMicro) * version;
	}
	static PointI DimensionOfVersionRMQR(int versionNumber);

	static bool HasMicroSize(const BitMatrix& bitMatrix);
	static bool HasRMQRSize(const BitMatrix& bitMatrix);
	static bool HasValidSize(const BitMatrix& bitMatrix);
	static int Number(const BitMatrix& bitMatrix);

	static const Version* DecodeVersionInformation(int versionBitsA, int versionBitsB = 0);

	static const Version* Model1(int number);
	static const Version* Model2(int number);
	static const Version* Micro(int number);
	static const Version* rMQR(int number);

private:
	int _versionNumber;
	std::vector<int> _alignmentPatternCenters;
	std::array<ECBlocks, 4> _ecBlocks;
	int _totalCodewords;
	Type _type;

	Version(int versionNumber, std::initializer_list<int> alignmentPatternCenters, const std::array<ECBlocks, 4> &ecBlocks);
	Version(int versionNumber, const std::array<ECBlocks, 4>& ecBlocks);
};

} // QRCode
} // ZXing
