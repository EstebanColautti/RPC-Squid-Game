@echo off
setlocal
cd /d "%~dp0"
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
echo Opening server and two client windows...
start "RPC Server" cmd /k build\server.exe 5050 10 90
timeout /t 1 >nul
start "RPC Client 1" cmd /k build\client.exe 127.0.0.1 5050
start "RPC Client 2" cmd /k build\client.exe 127.0.0.1 5050
