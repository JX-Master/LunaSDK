@echo off
setlocal

dotnet run --project "%~dp0Tools\LunaSetup\LunaSetup.csproj" -- --root "%~dp0." %* --platform windows
exit /b %ERRORLEVEL%
