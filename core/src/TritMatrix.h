#pragma once
/*
* Copyright 2020 Axel Waggershauser
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

#include "Matrix.h"

#include <cstdint>

namespace ZXing {

/**
 * @brief Represent a tri-state value false/true/empty
 */
class Trit
{
public:
	enum value_t : uint8_t {false_v, true_v, empty_v} value = empty_v;
	Trit() = default;
	Trit(bool v) : value(static_cast<value_t>(v)) {}
	operator bool() const { return value == true_v; }
	bool isEmpty() const { return value == empty_v; }
};

using TritMatrix = Matrix<Trit>;

} // ZXing
