@echo off
call BUILD_WINDOWS.bat
if errorlevel 1 exit /b 1
start "RPC Squid Game Server" cmd /k build\server.exe 5050 10 90
timeout /t 1 >nul
start "RPC Squid Game Client 1" cmd /k build\client.exe 127.0.0.1 5050
start "RPC Squid Game Client 2" cmd /k build\client.exe 127.0.0.1 5050
