@echo off
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
build\demo.exe
pause
