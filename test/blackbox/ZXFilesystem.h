/*
* Copyright 2019 Axel Waggershauser.
*/
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if __has_include(<filesystem>)
#  include <filesystem>
#  ifdef __cpp_lib_filesystem
     namespace fs = std::filesystem;
#  endif
#endif
#if !defined(__cpp_lib_filesystem) && __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
#  ifdef __cpp_lib_experimental_filesystem
     namespace fs = std::experimental::filesystem;
#  endif
#endif

#if !defined(__cpp_lib_filesystem) && !defined(__cpp_lib_experimental_filesystem)
#  error need standard filesystem library from c++-17 or the Filesystem TR
#endif

// compiling this with clang (e.g. version 6) might require linking against libc++experimental.a or libc++fs.a.
// E.g.: CMAKE_EXE_LINKER_FLAGS = -L/usr/local/Cellar/llvm/6.0.1/lib -lc++experimental
