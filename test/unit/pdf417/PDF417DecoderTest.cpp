/*
* Copyright 2017 Huy Cuong Nguyen
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "DecoderResult.h"
#include "DecodeStatus.h"
#include "pdf417/PDFDecodedBitStreamParser.h"
#include "pdf417/PDFDecoderResultExtra.h"

#include "gtest/gtest.h"

namespace ZXing { namespace Pdf417 {
	DecodeStatus DecodeMacroBlock(const std::vector<int>& codewords, int codeIndex, DecoderResultExtra& resultMetadata, int& next);
}}

using namespace ZXing;
using namespace ZXing::Pdf417;

/**
* Tests the first sample given in ISO/IEC 15438:2015(E) - Annex H.4
*/
TEST(PDF417DecoderTest, StandardSample1)
{
	std::vector<int> sampleCodes = { 20, 928, 111, 100, 17, 53, 923, 1, 111, 104, 923, 3, 64, 416, 34, 923, 4, 258, 446, 67,
		// we should never reach these
		1000, 1000, 1000 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 2, resultMetadata, next);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("017053", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());
	EXPECT_EQ(4, resultMetadata.segmentCount());
	EXPECT_EQ("CEN BE", resultMetadata.sender());
	EXPECT_EQ("ISO CH", resultMetadata.addressee());

	auto optionalData = resultMetadata.optionalData();
	EXPECT_EQ(1, optionalData.front()) << "first element of optional array should be the first field identifier";
	EXPECT_EQ(67, optionalData.back()) << "last element of optional array should be the last codeword of the last field";

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("017053", result.structuredAppend().id);
	EXPECT_EQ(4, result.structuredAppend().count);
}

/**
* Tests the second given in ISO/IEC 15438:2015(E) - Annex H.4
*/
TEST(PDF417DecoderTest, StandardSample2)
{
	std::vector<int> sampleCodes = { 11, 928, 111, 103, 17, 53, 923, 1, 111, 104, 922,
		// we should never reach these
		1000, 1000, 1000 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 2, resultMetadata, next);

	EXPECT_EQ(3, resultMetadata.segmentIndex());
	EXPECT_EQ("017053", resultMetadata.fileId());
	EXPECT_EQ(true, resultMetadata.isLastSegment());
	EXPECT_EQ(4, resultMetadata.segmentCount());
	EXPECT_EQ("", resultMetadata.sender());
	EXPECT_EQ("", resultMetadata.addressee());

	auto optionalData = resultMetadata.optionalData();
	EXPECT_EQ(1, optionalData.front()) << "first element of optional array should be the first field identifier";
	EXPECT_EQ(104, optionalData.back()) << "last element of optional array should be the last codeword of the last field";

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(3, result.structuredAppend().index);
	EXPECT_EQ("017053", result.structuredAppend().id);
	EXPECT_EQ(4, result.structuredAppend().count);
}

/**
* Tests the example given in ISO/IEC 15438:2015(E) - Annex H.6
*/
TEST(PDF417DecoderTest, StandardSample3)
{
	// Note final dummy ECC codeword required for DecodedBitStreamParser::Decode() to work
	std::vector<int> sampleCodes = { 7, 928, 111, 100, 100, 200, 300, 0 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 2, resultMetadata, next);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("100200300", resultMetadata.fileId());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("100200300", result.structuredAppend().id);
	EXPECT_EQ(0, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithFilename)
{
	std::vector<int> sampleCodes = { 23, 477, 928, 111, 100, 0, 252, 21, 86, 923, 0, 815, 251, 133, 12, 148, 537, 593,
		599, 923, 1, 111, 102, 98, 311, 355, 522, 920, 779, 40, 628, 33, 749, 267, 506, 213, 928, 465, 248, 493, 72,
		780, 699, 780, 493, 755, 84, 198, 628, 368, 156, 198, 809, 19, 113 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 3, resultMetadata, next);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("000252021086", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());
	EXPECT_EQ(2, resultMetadata.segmentCount());
	EXPECT_EQ("", resultMetadata.sender());
	EXPECT_EQ("", resultMetadata.addressee());
	EXPECT_EQ("filename.txt", resultMetadata.fileName());

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("000252021086", result.structuredAppend().id);
	EXPECT_EQ(2, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithNumericValues)
{
	std::vector<int> sampleCodes = { 25, 477, 928, 111, 100, 0, 252, 21, 86, 923, 2, 2, 0, 1, 0, 0, 0, 923, 5, 130, 923, 6, 1, 500, 13, 0 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 3, resultMetadata, next);

	EXPECT_EQ(0, resultMetadata.segmentIndex());
	EXPECT_EQ("000252021086", resultMetadata.fileId());
	EXPECT_EQ(false, resultMetadata.isLastSegment());

	EXPECT_EQ(180980729000000L, resultMetadata.timestamp());
	EXPECT_EQ(30, resultMetadata.fileSize());
	EXPECT_EQ(260013, resultMetadata.checksum());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(0, result.structuredAppend().index);
	EXPECT_EQ("000252021086", result.structuredAppend().id);
	EXPECT_EQ(0, result.structuredAppend().count);
}

TEST(PDF417DecoderTest, SampleWithMacroTerminatorOnly)
{
	std::vector<int> sampleCodes = { 7, 477, 928, 222, 198, 0, 922, 0 };

	int next = 0;
	DecoderResultExtra resultMetadata;
	auto status = DecodeMacroBlock(sampleCodes, 3, resultMetadata, next);

	EXPECT_EQ(99998, resultMetadata.segmentIndex());
	EXPECT_EQ("000", resultMetadata.fileId());
	EXPECT_EQ(true, resultMetadata.isLastSegment());
	EXPECT_EQ(-1, resultMetadata.segmentCount());

	auto result = DecodedBitStreamParser::Decode(sampleCodes, 0 /*ecLevel*/);

	EXPECT_EQ(99998, result.structuredAppend().index);
	EXPECT_EQ("000", result.structuredAppend().id);
	EXPECT_EQ(99999, result.structuredAppend().count);
}
