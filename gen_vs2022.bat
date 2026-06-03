@echo off
setlocal

dotnet run --project "%~dp0Tools\LunaBuild\src\LunaBuild.Cli\LunaBuild.Cli.csproj" -- generate --root "%~dp0." --format vs2022 --all --platform Windows --arch x64
pause
