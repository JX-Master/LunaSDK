#!/usr/bin/env sh
set -eu

PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

if [ ! -d "$PROJECT_ROOT/SDKs" ]; then
    echo "LunaSDK requires third-party SDK files under $PROJECT_ROOT/SDKs."
    echo "LunaBuild does not download packages. Please prepare SDKs before building."
    exit 1
fi

echo "SDKs directory found. LunaBuild is ready to generate and build targets."
