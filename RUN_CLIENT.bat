@echo off
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
build\client.exe 127.0.0.1 5050
pause
