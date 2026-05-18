@echo off
setlocal
cd /d "%~dp0"
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
echo.
echo Running concurrent demo...
echo.
build\concurrent_demo.exe
echo.
pause
