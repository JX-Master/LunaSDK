#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../../.." && pwd)
LLVM_SDK="$REPO_ROOT/SDKs/llvm-21.1.1/macosx/arm64"

mkdir -p "$SCRIPT_DIR/bin"

clang++ -std=c++20 -fno-rtti \
  -I "$LLVM_SDK/include" \
  "$SCRIPT_DIR/src/main.cpp" \
  -L "$LLVM_SDK/lib" \
  -lclang-cpp \
  -Wl,-rpath,"$LLVM_SDK/lib" \
  -framework CoreServices \
  -framework CoreFoundation \
  -o "$SCRIPT_DIR/bin/cppsl-native-extractor"
