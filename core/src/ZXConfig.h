/*
* Copyright 2016 Nu-book Inc.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define ZX_HAVE_CONFIG

// Thread local or static memory may be used to reduce the number of (re-)allocations of temporary variables
// in e.g. the ReedSolomonDecoder. It is disabled by default. It can be enabled by modifying the following define.
// Note: The Apple clang compiler until XCode 8 does not support c++11's thread_local.
// The alternative 'static' makes the code thread unsafe.
#define ZX_THREAD_LOCAL // 'thread_local' or 'static'

// The two basic data structures for storing bits (BitArray and BitMatrix) can be operated by storing one bit
// of information either in one bit or one byte. Storing it in one byte is considerably faster, while obviously
// using more memory. The effect of the memory usage while running the TestRunner is virtually invisible.
// On embedded/mobile systems this might be of importance. Note: the BitMatrix in 'fast' mode still requires
// only 1/3 of the same image in RGB.
#define ZX_FAST_BIT_STORAGE // undef to disable

// The Galoir Field abstractions used in Reed-Solomon error correction code can use more memory to eliminate a modulo
// operation. This improves performance but might not be the best option if RAM is scarce. The effect is a few kB big.
#define ZX_REED_SOLOMON_USE_MORE_MEMORY_FOR_SPEED
