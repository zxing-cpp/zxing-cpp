# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-src")
  file(MAKE_DIRECTORY "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-src")
endif()
file(MAKE_DIRECTORY
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-build"
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix"
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/tmp"
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp"
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/src"
  "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/run/media/leemu/D8C48579C4855B20/projects/zxing-cpp/build/_deps/stb-subbuild/stb-populate-prefix/src/stb-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
