#!/bin/bash
set -e -x

# Use static path to the zxing-cpp core directory instead of relative path from python directory because the
# `pip wheel .` works in a temporary directory leading to unknown directory for ${CMAKE_CURRENT_SOURCE_DIR}/../../core

sed -i "s:\${CMAKE_CURRENT_SOURCE_DIR}/\.\./\.\./core:$(pwd)/../../core:" CMakeLists.txt
