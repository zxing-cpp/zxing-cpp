/*
* Copyright 2018 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Thread local or static memory may be used to reduce the number of (re-)allocations of temporary variables
// in e.g. the ReedSolomonDecoder. It is disabled by default. It can be enabled by modifying the following define.
// Note: The Apple clang compiler until XCode 8 does not support c++11's thread_local.
// The alternative 'static' makes the code thread unsafe.
#define ZX_THREAD_LOCAL // '' (nothing), 'thread_local' or 'static'

// The Galoir Field abstractions used in Reed-Solomon error correction code can use more memory to eliminate a modulo
// operation. This improves performance but might not be the best option if RAM is scarce. The effect is a few kB big.
#define ZX_REED_SOLOMON_USE_MORE_MEMORY_FOR_SPEED
