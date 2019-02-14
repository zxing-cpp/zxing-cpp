#pragma once
/*
* Copyright 2016 Nu-book Inc.
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

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#define ZX_HAVE_CONFIG

// Version numbering
#define ZXING_VERSION_MAJOR 1
#define ZXING_VERSION_MINOR 0
#define ZXING_VERSION_PATCH 3

#if !__has_attribute(cxx_rtti) && !defined(__RTTI) && !defined(_CPPRTTI) && !defined(__GXX_RTTI) && !defined(__INTEL_RTTI__)
	#define ZX_NO_RTTI
#endif

// Thread local or static memory may be used to reduce the number of (re-)allocations of temporary variables
// in e.g. the ReedSolomonDecoder. It is desabled by default. It can be enabled by modifying the following define.
// Note: The Apple clang compiler until XCode 8 does not support c++11's thread_local.
// The alternative 'static' makes the code thread unsafe.
#define ZX_THREAD_LOCAL // 'thread_local' or 'static'
