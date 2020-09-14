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

#include <utility>

namespace ZXing {

/**
 * ScopeExit is a trivial helper meant to be later replaced by std::scope_exit from library fundamentals TS v3.
 */

template <typename EF>
class ScopeExit
{
	EF fn;
public:
	ScopeExit(EF&& fn) : fn(std::move(fn)) {}
	~ScopeExit() { fn(); }
};

template <class EF>
ScopeExit(EF) -> ScopeExit<EF>;

/**
 * The SCOPE_EXIT macro is eliminating the need to give the object a name.
 * Example usage:
 *   SCOPE_EXIT([]{ printf("exiting scope"); });
 */

#define SCOPE_EXIT_CAT2(x, y) x##y
#define SCOPE_EXIT_CAT(x, y) SCOPE_EXIT_CAT2(x, y)
#define SCOPE_EXIT const auto SCOPE_EXIT_CAT(scopeExit_, __COUNTER__) = ScopeExit

} // ZXing

