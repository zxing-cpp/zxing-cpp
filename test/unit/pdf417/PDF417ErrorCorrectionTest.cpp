/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2012 ZXing authors
* Copyright 2026 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "PseudoRandom.h"
#include "ZXAlgorithms.h"
#include "ReedSolomon.h"

#include "gtest/gtest.h"

using namespace ZXing;

static const std::vector<int> PDF417_TEST = {
	48, 901, 56, 141, 627, 856, 330, 69, 244, 900, 852, 169, 843, 895, 852, 895, 913, 154, 845, 778, 387, 89, 869,
	901, 219, 474, 543, 650, 169, 201, 9, 160, 35, 70, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900 };
static const std::vector<int> PDF417_TEST_WITH_EC{
	48, 901, 56, 141, 627, 856, 330, 69, 244, 900, 852, 169, 843, 895, 852, 895, 913, 154, 845, 778, 387, 89, 869,
	901, 219, 474, 543, 650, 169, 201, 9, 160, 35, 70, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900, 900,
	900, 900, 769, 843, 591, 910, 605, 206, 706, 917, 371, 469, 79, 718, 47, 777, 249, 262, 193, 620, 597, 477, 450,
	806, 908, 309, 153, 871, 686, 838, 185, 674, 68, 679, 691, 794, 497, 479, 234, 250, 496, 43, 347, 582, 882, 536,
	322, 317, 273, 194, 917, 237, 420, 859, 340, 115, 222, 808, 866, 836, 417, 121, 833, 459, 64, 159 };
static const int NUM_ECC = Size(PDF417_TEST_WITH_EC) - Size(PDF417_TEST);
static const int ERROR_LIMIT = NUM_ECC;
static const int MAX_ERRORS = ERROR_LIMIT / 2;
static const int MAX_ERASURES = ERROR_LIMIT;

static void CheckDecode(std::vector<int>& codeword, std::span<int> erasures = {}, int i = 0)
{
	auto res = ReedSolomonDecode(RSField::PDF417, codeword, NUM_ECC, erasures);
	EXPECT_TRUE(res) << "Failed to correct at " << i << " with " << erasures.size() << " erasures";
	EXPECT_EQ(codeword, PDF417_TEST_WITH_EC);
}

static std::vector<int> Corrupt(std::vector<int>& codeword, int howMany, PseudoRandom& random, int max)
{
	std::vector<bool> corrupted(codeword.size(), false);
	for (int j = 0; j < howMany; j++) {
		int location = random.next(0, Size(codeword) - 1);
		int value = random.next(0, max - 1);
		if (corrupted[location] || codeword[location] == value) {
			j--;
		}
		else {
			corrupted[location] = true;
			codeword[location] = value;
		}
	}
	std::vector<int> erasures;
	for (size_t i = 0; i < codeword.size(); i++)
		if (corrupted[i])
			erasures.push_back(i);

	return erasures;
}

TEST(PDF417ErrorCorrectionTest, NoError)
{
	std::vector<int> codeword(PDF417_TEST_WITH_EC);
	// no errors
	CheckDecode(codeword);
}

TEST(PDF417ErrorCorrectionTest, OneError)
{
	PseudoRandom random(0x12345678);
	for (int i = 0; i < Size(PDF417_TEST_WITH_EC); i++) {
		std::vector<int> codeword(PDF417_TEST_WITH_EC);
		codeword[i] = random.next(0, 255);
		CheckDecode(codeword, {}, i);
	}
}

TEST(PDF417ErrorCorrectionTest, OneErasure)
{
	for (int i = 0; i < Size(PDF417_TEST_WITH_EC); i++) {
		std::vector<int> codeword(PDF417_TEST_WITH_EC);
		std::vector<int> erasures = {i};
		codeword[i] = 0;
		CheckDecode(codeword, erasures, i);
	}
}

TEST(PDF417ErrorCorrectionTest, OneErasureOneError)
{
	std::vector<int> codeword(PDF417_TEST_WITH_EC);
	std::vector<int> erasures = {5};
	for (int i : {5, 10})
		codeword[i] = 0;
	CheckDecode(codeword, erasures);
}

TEST(PDF417ErrorCorrectionTest, MaxErrors)
{
	PseudoRandom random(0x12345678);
	for (int testIterations = 0; testIterations < 100; testIterations++) { // # iterations is kind of arbitrary
		std::vector<int> codeword(PDF417_TEST_WITH_EC);
		Corrupt(codeword, MAX_ERRORS, random, 929);
		CheckDecode(codeword);
	}
}

TEST(PDF417ErrorCorrectionTest, MaxErasures)
{
	PseudoRandom random(0x12345678);
	for (int testIterations = 0; testIterations < 1; testIterations++) { // # iterations is kind of arbitrary
		std::vector<int> codeword(PDF417_TEST_WITH_EC);
		auto erasures = Corrupt(codeword, MAX_ERASURES, random, 929);
		CheckDecode(codeword, erasures);

		while (erasures.size() > 1) {
			// replace 2 erasures with 1 error
			codeword[erasures.back()] = PDF417_TEST_WITH_EC[erasures.back()]; // un-corrupt one erasure to make it correctable
			erasures.pop_back();
			erasures.pop_back();
			CheckDecode(codeword, erasures);
		}
	}
}

TEST(PDF417ErrorCorrectionTest, Errata)
{
	std::vector<int> codeword(PDF417_TEST_WITH_EC);
	int numECC = Size(PDF417_TEST_WITH_EC) - Size(PDF417_TEST);
	std::vector<int> erasures;
	for (size_t i = 0; i < numECC; i++) {
		auto received = codeword;
		std::fill_n(received.begin(), i + 1 + (i < numECC / 2) + (i < numECC / 3), 1);
		erasures.push_back(i);
		CheckDecode(received, erasures, i);
	}
}
TEST(PDF417ErrorCorrectionTest, TooManyErrors)
{
	std::vector<int> codeword(PDF417_TEST_WITH_EC);
	PseudoRandom random(0x12345678);
	Corrupt(codeword, MAX_ERRORS + 1, random, 929);
	EXPECT_FALSE(ReedSolomonDecode(RSField::PDF417, codeword, NUM_ECC));
}
