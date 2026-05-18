#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

CONFIGURATION="${1:-Debug}"
CLI_OUTPUT="$SCRIPT_DIR/src/CPPSL.Cli/bin/$CONFIGURATION/net9.0"
NATIVE_EXTRACTOR="$SCRIPT_DIR/native/bin/cppsl-native-extractor"
SDK_BIN="$REPO_ROOT/SDKs/CPPSL/macosx/arm64/bin"

if [[ ! -d "$CLI_OUTPUT" ]]; then
    echo "error: CPPSL CLI output does not exist: $CLI_OUTPUT" >&2
    echo "build it first, for example: DOTNET_CLI_HOME=/tmp dotnet build Tools/CPPSL/src/CPPSL.Cli/CPPSL.Cli.csproj -c $CONFIGURATION -m:1 /nr:false" >&2
    exit 1
fi

if [[ ! -x "$CLI_OUTPUT/cppslc" ]]; then
    echo "error: cppslc does not exist or is not executable: $CLI_OUTPUT/cppslc" >&2
    exit 1
fi

if [[ ! -x "$NATIVE_EXTRACTOR" ]]; then
    echo "error: CPPSL native extractor does not exist or is not executable: $NATIVE_EXTRACTOR" >&2
    echo "build it first with the native extractor build script or LunaBuild target for this platform" >&2
    exit 1
fi

mkdir -p "$SDK_BIN"
cp -R "$CLI_OUTPUT/." "$SDK_BIN/"
cp "$NATIVE_EXTRACTOR" "$SDK_BIN/cppsl-native-extractor"
chmod +x "$SDK_BIN/cppslc" "$SDK_BIN/cppsl-native-extractor"

echo "CPPSL macOS arm64 artifacts copied to: $SDK_BIN"
