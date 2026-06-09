#!/usr/bin/env sh
set -eu

PROJECT_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

dotnet run --project "$PROJECT_ROOT/Tools/LunaSetup/LunaSetup.csproj" -- --root "$PROJECT_ROOT" "$@"
