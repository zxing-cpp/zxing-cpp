#!/bin/sh

EMSCRIPTEN_PATH=~/emsdk/emscripten/tag-1.38.21
SOURCE_BASEDIR=$(dirname "$0")

"$EMSCRIPTEN_PATH/emconfigure" cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$SOURCE_BASEDIR/Toolchain-Emscripten.cmake" -DEMSCRIPTEN_ROOT_PATH="$EMSCRIPTEN_PATH" "$SOURCE_BASEDIR"
"$EMSCRIPTEN_PATH/emmake" make
