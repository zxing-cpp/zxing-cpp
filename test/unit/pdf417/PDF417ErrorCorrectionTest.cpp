/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2012 ZXing authors
*/
// SPDX-License-Identifier: Apache-2.0

#include "PseudoRandom.h"
#include "ZXAlgorithms.h"

#include "gtest/gtest.h"

namespace ZXing {
	namespace Pdf417 {
		bool DecodeErrorCorrection(std::vector<int>& received, int numECCodewords, const std::vector<int>& erasures, int& nbErrors);
	}
}

using namespace ZXing;
using namespace ZXing::Pdf417;

static const int PDF417_TEST[] = {
	48, 901, 56, 141, 627, 856, 330, 69, 244, 900, 852, 169, 843, 895, 852, 895, 913, 154, 845, 778, 387, 89, 869,
	901, 219, 474, 543, 650, 169, 201, 9, 160, 35, 70, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900 };
static const int PDF417_TEST_WITH_EC[] = {
	48, 901, 56, 141, 627, 856, 330, 69, 244, 900, 852, 169, 843, 895, 852, 895, 913, 154, 845, 778, 387, 89, 869,
	901, 219, 474, 543, 650, 169, 201, 9, 160, 35, 70, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900, 769, 843, 591, 910, 605, 206, 706, 917, 371, 469, 79, 718, 47, 777, 249, 262, 193, 620, 597, 477, 450,
	806, 908, 309, 153, 871, 686, 838, 185, 674, 68, 679, 691, 794, 497, 479, 234, 250, 496, 43, 347, 582, 882, 536,
	322, 317, 273, 194, 917, 237, 420, 859, 340, 115, 222, 808, 866, 836, 417, 121, 833, 459, 64, 159 };
static const int ECC_BYTES = Size(PDF417_TEST_WITH_EC) - Size(PDF417_TEST);
static const int ERROR_LIMIT = ECC_BYTES;
static const int MAX_ERRORS = ERROR_LIMIT / 2;
static const int MAX_ERASURES = ERROR_LIMIT;

static void CheckDecode(std::vector<int>& received, const std::vector<int>& erasures)
{
	int nbError = 0;
	bool corrected = DecodeErrorCorrection(received, ECC_BYTES, erasures, nbError);
	EXPECT_TRUE(corrected);
	for (int i = 0; i < Size(PDF417_TEST); i++) {
		EXPECT_EQ(received[i], PDF417_TEST[i]);
	}
}

static void CheckDecode(std::vector<int>& received)
{
	CheckDecode(received, std::vector<int>());
}

static void Corrupt(std::vector<int>& received, int howMany, PseudoRandom& random, int max)
{
	std::vector<bool> corrupted(received.size(), false);
	for (int j = 0; j < howMany; j++) {
		int location = random.next(0, Size(received) - 1);
		int value = random.next(0, max - 1);
		if (corrupted[location] || received[location] == value) {
			j--;
		}
		else {
			corrupted[location] = true;
			received[location] = value;
		}
	}
}


TEST(PDF417ErrorCorrectionTest, NoError)
{
	std::vector<int> received(PDF417_TEST_WITH_EC, PDF417_TEST_WITH_EC + Size(PDF417_TEST_WITH_EC));
	// no errors
	CheckDecode(received);
}

TEST(PDF417ErrorCorrectionTest, OneError)
{
	PseudoRandom random(0x12345678);
	for (int i = 0; i < Size(PDF417_TEST_WITH_EC); i++) {
		std::vector<int> received(PDF417_TEST_WITH_EC, PDF417_TEST_WITH_EC + Size(PDF417_TEST_WITH_EC));
		received[i] = random.next(0, 255);
		CheckDecode(received);
	}
}

TEST(PDF417ErrorCorrectionTest, MaxErrors)
{
	PseudoRandom random(0x12345678);
	for (int testIterations = 0; testIterations < 100; testIterations++) { // # iterations is kind of arbitrary
		std::vector<int> received(PDF417_TEST_WITH_EC, PDF417_TEST_WITH_EC + Size(PDF417_TEST_WITH_EC));
		Corrupt(received, MAX_ERRORS, random, 929);
		CheckDecode(received);
	}
}

TEST(PDF417ErrorCorrectionTest, TooManyErrors)
{
	std::vector<int> received(PDF417_TEST_WITH_EC, PDF417_TEST_WITH_EC + Size(PDF417_TEST_WITH_EC));
	PseudoRandom random(0x12345678);
	Corrupt(received, MAX_ERRORS + 1, random, 929);
	int nbError = 0;
	EXPECT_FALSE(DecodeErrorCorrection(received, ECC_BYTES, std::vector<int>(), nbError));
}
