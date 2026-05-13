@echo off
setlocal

if not exist "%~dp0SDKs" (
    echo LunaSDK requires third-party SDK files under "%~dp0SDKs".
    echo LunaBuild does not download packages. Please prepare SDKs before building.
    exit /b 1
)

echo SDKs directory found. LunaBuild is ready to generate and build targets.
