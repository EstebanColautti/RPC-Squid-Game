@echo off
setlocal
cd /d "%~dp0"
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
echo.
echo Running automatic demo...
echo.
build\demo.exe
echo.
pause
