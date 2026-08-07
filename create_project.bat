@echo off
setlocal

dotnet run --project "%~dp0Tools\LunaProjectGenerator\LunaProjectGenerator.csproj" -- --sdk-root "%~dp0." %*
exit /b %ERRORLEVEL%
