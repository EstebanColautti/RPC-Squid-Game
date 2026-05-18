@echo off
setlocal
cd /d "%~dp0"
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
build\server.exe 5050 10 90
pause
