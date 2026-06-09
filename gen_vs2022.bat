@echo off
setlocal

dotnet run --project "%~dp0LunaBuild.csproj" -- generate --root "%~dp0." --format vs2022 --all --platform Windows --arch x64
pause
