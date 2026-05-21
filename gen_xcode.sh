#!/usr/bin/env sh
set -eu

PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

dotnet run --project "$PROJECT_ROOT/Tools/LunaBuild/src/LunaBuild.Cli/LunaBuild.Cli.csproj" -- generate --root "$PROJECT_ROOT" --format xcode --all --platform MacOS --arch arm64
