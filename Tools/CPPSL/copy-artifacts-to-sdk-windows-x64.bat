@echo off
setlocal

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0copy-artifacts-to-sdk-windows-x64.ps1" %*
exit /b %ERRORLEVEL%
