/*
* Copyright 2016 Nu-book Inc.
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

#include "oned/rss/ODRSSFieldParser.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

#include <cstdlib>
#include <algorithm>

namespace ZXing {
namespace OneD {
namespace RSS {

	//private static final Object VARIABLE_LENGTH = new Object();

struct AiInfo
{
	const char* aiPrefix;
	int fieldSize;	// if negative, the length is variable and abs(length) give the max size
};

static const AiInfo aiInfos[] = {
// TWO_DIGIT_DATA_LENGTH
	{ "00", 18 },
	{ "01", 14 },
	{ "02", 14 },

	{ "10", -20 },
	{ "11", 6 },
	{ "12", 6 },
	{ "13", 6 },
	{ "15", 6 },
	{ "17", 6 },

	{ "20", 2 },
	{ "21", -20 },
	{ "22", -29 },

	{ "30", -8 },
	{ "37", -8 },

	//internal company codes
	{ "90", -30 },
	{ "91", -30 },
	{ "92", -30 },
	{ "93", -30 },
	{ "94", -30 },
	{ "95", -30 },
	{ "96", -30 },
	{ "97", -30 },
	{ "98", -30 },
	{ "99", -30 },

//THREE_DIGIT_DATA_LENGTH
	{ "240", -30 },
	{ "241", -30 },
	{ "242", -6 },
	{ "250", -30 },
	{ "251", -30 },
	{ "253", -17 },
	{ "254", -20 },

	{ "400", -30 },
	{ "401", -30 },
	{ "402", 17 },
	{ "403", -30 },
	{ "410", 13 },
	{ "411", 13 },
	{ "412", 13 },
	{ "413", 13 },
	{ "414", 13 },
	{ "420", -20 },
	{ "421", -15 },
	{ "422", 3 },
	{ "423", -15 },
	{ "424", 3 },
	{ "425", 3 },
	{ "426", 3 },

//THREE_DIGIT_PLUS_DIGIT_DATA_LENGTH
	{ "310", 6 },
	{ "311", 6 },
	{ "312", 6 },
	{ "313", 6 },
	{ "314", 6 },
	{ "315", 6 },
	{ "316", 6 },
	{ "320", 6 },
	{ "321", 6 },
	{ "322", 6 },
	{ "323", 6 },
	{ "324", 6 },
	{ "325", 6 },
	{ "326", 6 },
	{ "327", 6 },
	{ "328", 6 },
	{ "329", 6 },
	{ "330", 6 },
	{ "331", 6 },
	{ "332", 6 },
	{ "333", 6 },
	{ "334", 6 },
	{ "335", 6 },
	{ "336", 6 },
	{ "340", 6 },
	{ "341", 6 },
	{ "342", 6 },
	{ "343", 6 },
	{ "344", 6 },
	{ "345", 6 },
	{ "346", 6 },
	{ "347", 6 },
	{ "348", 6 },
	{ "349", 6 },
	{ "350", 6 },
	{ "351", 6 },
	{ "352", 6 },
	{ "353", 6 },
	{ "354", 6 },
	{ "355", 6 },
	{ "356", 6 },
	{ "357", 6 },
	{ "360", 6 },
	{ "361", 6 },
	{ "362", 6 },
	{ "363", 6 },
	{ "364", 6 },
	{ "365", 6 },
	{ "366", 6 },
	{ "367", 6 },
	{ "368", 6 },
	{ "369", 6 },
	{ "390", -15 },
	{ "391", -18 },
	{ "392", -15 },
	{ "393", -18 },
	{ "703", -30 },

//FOUR_DIGIT_DATA_LENGTH
	{ "7001", 13 },
	{ "7002", -30 },
	{ "7003", 10 },

	{ "8001", 14 },
	{ "8002", -20 },
	{ "8003", -30 },
	{ "8004", -30 },
	{ "8005", 6 },
	{ "8006", 18 },
	{ "8007", -30 },
	{ "8008", -12 },
	{ "8018", 18 },
	{ "8020", -25 },
	{ "8100", 6 },
	{ "8101", 10 },
	{ "8102", 2 },
	{ "8110", -70 },
	{ "8200", -70 },
};

static size_t
AiSize(const char* aiPrefix)
{
	if ((aiPrefix[0] == '3' && Contains("1234569", aiPrefix[1])) || std::string(aiPrefix) == "703")
		return 4;
	else
		return strlen(aiPrefix);
}


DecodeStatus
FieldParser::ParseFieldsInGeneralPurpose(const std::string &rawInfo, std::string& result)
{
	if (rawInfo.empty()) {
		return DecodeStatus::NoError;
	}

	auto starts_with =
		[](const std::string& str, const char* pre) { return strncmp(pre, str.data(), strlen(pre)) == 0; };

	const AiInfo* aiInfo = FindIf(aiInfos, [&](const AiInfo& i) { return starts_with(rawInfo, i.aiPrefix); });
	if (aiInfo == std::end(aiInfos))
		return DecodeStatus::NotFound;

	size_t aiSize = AiSize(aiInfo->aiPrefix);

	// require at least one character in the variable field size case
	if (rawInfo.length() < aiSize + std::max(1, aiInfo->fieldSize))
		return DecodeStatus::NotFound;

	size_t fieldSize = aiInfo->fieldSize >= 0
						   ? size_t(aiInfo->fieldSize)                                        // fixed
						   : std::min(rawInfo.length() - aiSize, size_t(-aiInfo->fieldSize)); // variable

	auto ai = rawInfo.substr(0, aiSize);
	auto field = rawInfo.substr(aiSize, fieldSize);
	auto remaining = rawInfo.substr(aiSize + fieldSize);
	std::string parsedRemaining;
	auto status = FieldParser::ParseFieldsInGeneralPurpose(remaining, parsedRemaining);
	result = '(' + ai + ')' + field + parsedRemaining;
	return status;
}


} // RSS
} // OneD
} // ZXing
