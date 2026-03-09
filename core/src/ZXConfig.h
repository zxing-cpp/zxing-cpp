/*
* Copyright 2018 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Thread local or static memory may be used to reduce the number of (re-)allocations of temporary variables
// in e.g. the ReedSolomonDecoder. The default is thread_local, which is the best option for performance and safety in most cases. If
// your platform doesn't support thread_local, you can switch to static, but be aware that this makes the code not thread safe.
// For Windows in Visual Studio 2019 on Intel 64-bit using thread_local causes a dependency to VCRUNTIME140_1.dll, so you need 2019
// runtime DLLs instead of only 2015 version.
#define ZX_THREAD_LOCAL thread_local // '' (nothing), 'thread_local' or 'static'

// The Galoir Field abstractions used in Reed-Solomon error correction code can use more memory to eliminate a modulo
// operation. This improves performance but might not be the best option if RAM is scarce. The effect is a few kB big.
#define ZX_REED_SOLOMON_USE_MORE_MEMORY_FOR_SPEED
