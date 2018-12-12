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

#include "oned/ODEANManufacturerOrgSupport.h"
#include "ZXStrConvWorkaround.h"

#include <algorithm>
#include <type_traits>
#include <string>

namespace ZXing {
namespace OneD {

struct CountryId
{
	int first;
	int last;
	const char *id;

	bool operator<(const CountryId &other) const {
		return first < other.first;
	}
};

static const CountryId COUNTRIES[] = {
	{0, 19, "US/CA"},
	{30, 39, "US"},
	{60, 139, "US/CA"},
	{300, 379, "FR"},
	{380, 380, "BG"},
	{383, 383, "SI"},
	{385, 385, "HR"},
	{387, 387, "BA"},
	{400, 440, "DE"},
	{450, 459, "JP"},
	{460, 469, "RU"},
	{471, 471, "TW"},
	{474, 474, "EE"},
	{475, 475, "LV"},
	{476, 476, "AZ"},
	{477, 477, "LT"},
	{478, 478, "UZ"},
	{479, 479, "LK"},
	{480, 480, "PH"},
	{481, 481, "BY"},
	{482, 482, "UA"},
	{484, 484, "MD"},
	{485, 485, "AM"},
	{486, 486, "GE"},
	{487, 487, "KZ"},
	{489, 489, "HK"},
	{490, 499, "JP"},
	{500, 509, "GB"},
	{520, 520, "GR"},
	{528, 528, "LB"},
	{529, 529, "CY"},
	{531, 531, "MK"},
	{535, 535, "MT"},
	{539, 539, "IE"},
	{540, 549, "BE/LU"},
	{560, 560, "PT"},
	{569, 569, "IS"},
	{570, 579, "DK"},
	{590, 590, "PL"},
	{594, 594, "RO"},
	{599, 599, "HU"},
	{600, 601, "ZA"},
	{603, 603, "GH"},
	{608, 608, "BH"},
	{609, 609, "MU"},
	{611, 611, "MA"},
	{613, 613, "DZ"},
	{616, 616, "KE"},
	{618, 618, "CI"},
	{619, 619, "TN"},
	{621, 621, "SY"},
	{622, 622, "EG"},
	{624, 624, "LY"},
	{625, 625, "JO"},
	{626, 626, "IR"},
	{627, 627, "KW"},
	{628, 628, "SA"},
	{629, 629, "AE"},
	{640, 649, "FI"},
	{690, 695, "CN"},
	{700, 709, "NO"},
	{729, 729, "IL"},
	{730, 739, "SE"},
	{740, 740, "GT"},
	{741, 741, "SV"},
	{742, 742, "HN"},
	{743, 743, "NI"},
	{744, 744, "CR"},
	{745, 745, "PA"},
	{746, 746, "DO"},
	{750, 750, "MX"},
	{754, 755, "CA"},
	{759, 759, "VE"},
	{760, 769, "CH"},
	{770, 770, "CO"},
	{773, 773, "UY"},
	{775, 775, "PE"},
	{777, 777, "BO"},
	{779, 779, "AR"},
	{780, 780, "CL"},
	{784, 784, "PY"},
	{785, 785, "PE"},
	{786, 786, "EC"},
	{789, 790, "BR"},
	{800, 839, "IT"},
	{840, 849, "ES"},
	{850, 850, "CU"},
	{858, 858, "SK"},
	{859, 859, "CZ"},
	{860, 860, "YU"},
	{865, 865, "MN"},
	{867, 867, "KP"},
	{868, 869, "TR"},
	{870, 879, "NL"},
	{880, 880, "KR"},
	{885, 885, "TH"},
	{888, 888, "SG"},
	{890, 890, "IN"},
	{893, 893, "VN"},
	{896, 896, "PK"},
	{899, 899, "ID"},
	{900, 919, "AT"},
	{930, 939, "AU"},
	{940, 949, "AZ"},
	{955, 955, "MY"},
	{958, 958, "MO"},
};

std::string
EANManufacturerOrgSupport::LookupCountryIdentifier(const std::string& productCode)
{
	int prefix = std::stoi(productCode.substr(0, 3));
	auto it = std::lower_bound(std::begin(COUNTRIES), std::end(COUNTRIES), CountryId{ prefix, 0, nullptr });
	if (it != std::end(COUNTRIES))
	{
		if (prefix >= it->first && it->last)
			return it->id;
	}
	return std::string();
}

} // OneD
} // ZXing
