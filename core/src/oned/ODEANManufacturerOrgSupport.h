#pragma once
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

#include <sstream>
#include <string>

namespace ZXing {
namespace OneD {

/**
* Records EAN prefix to GS1 Member Organization, where the member organization
* correlates strongly with a country. This is an imperfect means of identifying
* a country of origin by EAN-13 barcode value. See
* <a href="http://en.wikipedia.org/wiki/List_of_GS1_country_codes">
* http://en.wikipedia.org/wiki/List_of_GS1_country_codes</a>.
*
* @author Sean Owen
*/
class EANManufacturerOrgSupport
{
public:
	static std::string LookupCountryIdentifier(const std::string& productCode);
};

} // OneD
} // ZXing
